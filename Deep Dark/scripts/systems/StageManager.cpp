#include "StageManager.h"
#include <ranges>
#include <algorithm>
#include <format>

using json = nlohmann::json;

float random_float(float min, float max) {
	if (min == max) return min;

	static std::minstd_rand gen(std::random_device{}());
	std::uniform_real_distribution<float> dis(min, max);
	return dis(gen);
}

StageManager::StageManager(const json& stageJson, std::vector<std::string>& slotsJsonPaths) :
	stageRecorder(stageJson["lane_count"]), loadout(slotsJsonPaths), 
	stage(stageJson, stageRecorder) 
{
	partsCountText.setString(std::format("$0/{}", bagCap));
	partsCountText.setPosition({ 50,50 });
	bagUpgradeCostText.setString(std::format("${}", bagUpgradeCost));
	bagUpgradeCostText.setPosition({ 50,700 });

	int challengeCount = stageJson["challenges"].size();
	challenges.reserve(challengeCount);
	for (auto& ch : stageJson["challenges"]) {
		std::string desc = ch["description"].get<std::string>();
		bool startState = ch.value("starting_state", false);

		std::string chaTypeStr = ch["challenge_type"].get<std::string>();
		char compChar = ch["comparison"].get<std::string>()[0];
		int team = ch.value("team", 0);
		int val = ch.value("value", 0);
		int val2 = ch.value("value2", 0);

		challenges.emplace_back(desc, chaTypeStr, compChar, team, val, val2);
		challenges.back().cleared = startState;
	}

	clearedChallengesText.setString(
		std::format("Challenges Cleared: 0/{}", challenges.size()));
	clearedChallengesText.setPosition({ 600,700 });
}

// Reading Inputs
bool StageManager::read_lane_switch_inputs(Key key) {
	if (key == Key::Up) {
		selectedLane = (selectedLane + 1) % stage.laneCount;
		return true;
	}
	else if (key == Key::Down) {
		selectedLane = (selectedLane - 1 + stage.laneCount) % stage.laneCount;
		return true;
	}

	stage.selectedLane = selectedLane;
	return false;
}
bool StageManager::read_pouch_upgrade_input(Key key) {
	if (currentBagLevel >= MAX_BAG_LEVEL) return false;
	else if (key == Key::B && try_spend_parts(bagUpgradeCost)) {
		currentBagLevel++;

		partsPerSecond = (int)std::round(partsPerSecond * 1.1f);
		bagCap += (int)std::round(baseBagCap * 0.5f);
		bagUpgradeCost += (int)std::round(baseBagCap * 0.25f);

		bagUpgradeCostText.setString(std::format("${}", bagUpgradeCost));
		return true;
	}
	return false;
}
bool StageManager::read_spawn_inputs(Key key) {
	for (int i = 0; i < loadout.filledSlots; i++) {
		if (key == numberKeys[i] && loadout.slots[i].cooldown <= 0) {
			selectedLane %= stage.laneCount;
			Slot& slot = loadout.slots[i];
			if (!try_spend_parts(slot.unitStats.parts)) return false;
			slot.cooldown = slot.spawnTimer;

			if (slot.unitStats.has_augment(DROP_BOX)) 
				create_drop_box(selectedLane, &slot.unitStats, &slot.aniMap);
			else
				stage.create_unit(selectedLane, &slot.unitStats, &slot.aniMap);

			return true;
		}
	}
	return false;
}
bool StageManager::read_base_fire_input(Key key) {
	if (key == Key::F) {
		stage.playerBase.fire_cannon();
		return true;
	}
	return false;
}
void StageManager::handle_events(sf::Event event) {
	if (auto keyEvent = event.getIf<sf::Event::KeyPressed>()) {
		if (read_spawn_inputs(keyEvent->code)) return;
		else if (read_lane_switch_inputs(keyEvent->code)) return;
		else if (read_base_fire_input(keyEvent->code)) return;
		else read_pouch_upgrade_input(keyEvent->code);
	}
}

// Action Objects
void StageManager::try_spawn_death_surge(Unit& unit) {
	if (!unit.trigger_augment(unit.stats, DEATH_SURGE)) return;
	const Augment& augment = unit.stats->get_augment(DEATH_SURGE);

	AugmentType surgeType = FIRE_WALL; 
	if (unit.stats->quickAugMask & SHOCK_WAVE) surgeType = SHOCK_WAVE;
	else if (unit.stats->quickAugMask & ORBITAL_STRIKE) surgeType = ORBITAL_STRIKE; 

	float distance = augment.value;
	Augment newSurge = Augment::surge(surgeType, distance, augment.surgeLevel, 0, unit.hitIndex);
	stage.create_surge(unit, newSurge);
}
void StageManager::create_drop_box(int laneInd, const UnitStats* stats, std::array<Animation, 5>* aniMap) {
	if (!stats->has_augment(DROP_BOX)) return;
	float percentage = stats->get_augment(DROP_BOX).value;
	Lane& lane = stage.lanes[laneInd];
	std::pair<float, float> range = { lane.playerXPos, lane.enemyXPos };
	float spawnPoint = range.first + (range.second - range.first) * percentage;

	/*
	std::cout << laneInd << ", lane y - pos: " << lane.yPos << ", lane range: ("
		<< range.first << " , " << range.second << ") percentage: " << percentage <<
		"spawnPointX: " << spawnPoint << std::endl;
		*/

	sf::Vector2f spawnPos = { spawnPoint, lane.yPos };
	ActionObjConfig config(stage, laneInd, spawnPos);
	stage.actionObjects.emplace_back(
		std::make_unique<UnitSpawner>(stats, aniMap, config));
}
void StageManager::try_create_cloner(Unit& unit) {
	if (!unit.has_augment(CLONE) || unit.statuses & CODE_BREAKER) return;

	ActionObjConfig config(stage, unit.currentLane, unit.pos);
	stage.actionObjects.emplace_back(
		std::make_unique<UnitSpawner>(unit.stats, unit.get_ani_array(), config));
}

void StageManager::collect_parts(Unit& unit) {
	int p = unit.stats->parts;
	if (unit.statuses & PLUNDER) p *= 2;
	gain_parts(p);
}
void StageManager::increment_parts_and_notify(float deltaTime) {
	partsIncTimer += deltaTime;
	while (partsIncTimer >= 1.0f) {
		gain_parts(partsPerSecond);
		notify_challenges();
		partsIncTimer -= 1.0f;
	}
}
void StageManager::handle_enemy_unit_death(Unit& unit) {
	if (unit.causeOfDeath != DeathCause::FALLING) {
		try_spawn_death_surge(unit);
		try_create_cloner(unit);
	}
	collect_parts(unit);
}
void StageManager::handle_player_unit_death(Unit& unit) {
	if (unit.causeOfDeath != DeathCause::FALLING) {
		try_spawn_death_surge(unit);
		try_create_cloner(unit);
	}
}
void StageManager::spawn_enemies(float deltaTime) {
	timeSinceStart += deltaTime;

	for (size_t i = stage.enemySpawners.size(); i--;) {
		auto& data = stage.enemySpawners[i];

		if (data.can_force_spawn(timeSinceStart)) {
			stage.create_unit(data.forcedSpawnTimes[0].second, &data.enemyStats, &data.aniArr);
			data.forcedSpawnTimes.erase(data.forcedSpawnTimes.begin());
		}

		if (data.currentSpawnIndex >= data.totalSpawns) continue;

		if (timeSinceStart > data.nextSpawnTime) {
			data.nextSpawnTime += random_float(data.spawnDelays.first, data.spawnDelays.second);
			int laneIndex = data.laneSpawnIndexes[data.currentSpawnIndex];
			stage.create_unit(laneIndex, &data.enemyStats, &data.aniArr);

			data.currentSpawnIndex++;
			if (data.currentSpawnIndex >= data.totalSpawns && data.infinite)
				data.currentSpawnIndex = 0;
		}
	}
}
void StageManager::process_move_requests() {
	for (size_t i = stage.moveRequests.size(); i--;) {
		auto& request = stage.moveRequests[i];
		std::cout << "proccessing move request, ID = " << request.unitId << std::endl;

		auto& sourceVec = stage.get_source_vector(request.currentLane, request.team);
		auto unit = std::find_if(sourceVec.begin(), sourceVec.end(),
			[id = request.unitId](const Unit& u) { return u.id == id; });

		if (unit == sourceVec.end() || request.currentLane == request.newLane) {
			stage.moveRequests.pop_back();
			std::cout << "ignoring move request" << std::endl;
			continue;
		}

		stage.cancel_tween(unit->id);

		auto& newVec = stage.get_source_vector(request.newLane, unit->stats->team);
		Unit& movedUnit = newVec.emplace_back(std::move(*unit));
		sourceVec.erase(unit);

		movedUnit.currentLane = request.newLane;

		if (request.fall_request()) movedUnit.fall(request.pos);
		else if (request.squash_request()) movedUnit.squash(request.pos);
		else if (request.jump_request()) movedUnit.jump(request.pos);
		else if (request.teleport_request()) {
			movedUnit.pos = { request.pos, stage.lanes[request.newLane].yPos };
		}
		else movedUnit.launch(request.pos);

		stage.moveRequests.pop_back();
	}
}
void StageManager::update_unit_ticks(sf::RenderWindow& window, float deltaTime) {
	//std::cout << "new update_unit_ticks() call" << std::endl;
	for (auto& lane : stage.lanes) {
		lane.draw(window);
		if (lane.trap) lane.trap->tick(window, deltaTime, stageRecorder);
		for (auto it = lane.enemyUnits.begin(); it != lane.enemyUnits.end();) {
			if (it->dead()) {
				handle_enemy_unit_death(*it);
				it = lane.enemyUnits.erase(it);
			}
			else {
				it->tick(window, deltaTime);
				++it;
			}
		}
		for (auto it = lane.playerUnits.begin(); it != lane.playerUnits.end();) {
			if (it->dead()) {
				handle_player_unit_death(*it);
				//std::cout << "Player Unit #" << it->id << " on Lane " << it->currentLane <<
				//	"is DEAD, erasing it from the vector" << std::endl;
				it = lane.playerUnits.erase(it);
			}
			else {
				it->tick(window, deltaTime);
				++it;
			}
		}
	}
}
void StageManager::update_ptr_ticks(sf::RenderWindow& window, float deltaTime) {
	for (auto it = stage.surges.begin(); it != stage.surges.end();) {
		if ((*it)->readyForRemoval) it = stage.surges.erase(it);
		else {
			(*it)->tick(window, deltaTime, stage);
			++it;
		}
	}
	for (auto it = stage.actionObjects.begin(); it != stage.actionObjects.end();) {
		if ((*it)->readyForRemoval) it = stage.actionObjects.erase(it);
		else {
			(*it)->tick(window, deltaTime);
			++it;
		}
	}
}
void StageManager::update_base_ticks(sf::RenderWindow& window, float deltaTime) {
	stage.playerBase.tick(stage, window, deltaTime);
	stage.enemyBase.tick(stage, window, deltaTime);
}

// UI
void StageManager::only_draw(sf::RenderWindow& window) {
	for (auto& lane : stage.lanes) {
		lane.draw(window);
		if (lane.trap) window.draw(lane.trap->sprite);
		for (auto it = lane.enemyUnits.begin(); it != lane.enemyUnits.end();) {
			window.draw(it->get_sprite());
			++it;
		}
		for (auto it = lane.playerUnits.begin(); it != lane.playerUnits.end();) {
			window.draw(it->get_sprite());
			++it;
		}
	}
}
void StageManager::update_ui(float deltaTime) {
	for (int i = 0; i < loadout.filledSlots; i++) 
		loadout.slots[i].cooldown -= deltaTime;

	partsCountText.setString(std::format("#{}/{}", parts, bagCap));
}
void StageManager::draw_ui() {
	loadout.draw_slots(cam, parts);

	cam->queue_ui_draw(&partsCountText);
	cam->queue_ui_draw(&bagUpgradeCostText);
	cam->queue_ui_draw(&clearedChallengesText);
}

void StageManager::update_game_ticks(sf::RenderWindow& window, float deltaTime) {
	spawn_enemies(deltaTime);
	update_unit_ticks(window, deltaTime); // This is where the erase is called
	update_ptr_ticks(window, deltaTime);
	update_base_ticks(window, deltaTime);
	process_move_requests(); // this is where the other erase is called
	increment_parts_and_notify(deltaTime);

	update_and_draw_ui(deltaTime);
}