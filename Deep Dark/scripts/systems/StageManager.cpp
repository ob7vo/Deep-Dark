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

void StageManager::create_stage(const json& stageJson) 
{
	int lanes = stageJson["lane_count"];
	challenges.clear();
	stageRecorder = StageRecord(lanes);
	stage = Stage(stageJson, &stageRecorder);

	bagCap = baseBagCap;
	bagUpgradeCost = baseBagUpgradeCost;

	ui.partsCountText.setString(std::format("$0/{}", bagCap));
	ui.bagUpgradeCostText.setString(std::format("${}", bagUpgradeCost));

	int challengeCount = stageJson["challenges"].size();
	challenges.reserve(challengeCount);
	for (auto& ch : stageJson["challenges"]) {
		std::string desc = ch.value("description", "");
		bool startState = ch.value("starting_state", false);

		std::string chaTypeStr = ch.value("challenge_type", "");
		char compChar = ch["comparison"].get<std::string>()[0];
		int team = ch.value("team", 0);
		int val = ch.value("value", 0);
		int lane = ch.value("lane", -1);

		if (ch.contains("banned_types"))
			for (auto& target : ch["banned_types"])
				val |= UnitStats::convert_string_to_type(target);

		Challenge& chal = challenges.emplace_back(desc, chaTypeStr, compChar, team, val, lane);
		chal.cleared = startState;
		chal.pTarget = chal.get_target_ptr(*this);
	}

	ui.clearedChallengesText.setString(
		std::format("Challenges Cleared: 0/{}", challenges.size()));
	ui.clearedChallengesText.setCharacterSize(16);
	ui.clearedChallengesText.setFillColor(sf::Color::Yellow);

	bagCap = baseBagCap;
	bagUpgradeCost = baseBagUpgradeCost;
}
void StageManager::unload_stage() {
	challenges.clear();
	stageRecorder = {};
	stage = {};

	clearedChallenges = 0;
	timeSinceStart = 0.f;
	selectedLane = 0;
	parts = 0;
	partsPerSecond = 5;
	currentBagLevel = 1;
	bagUpgradeCost = 20;
	baseBagUpgradeCost = 20;
	bagCap = 500;
	baseBagCap = 500;
	partsIncTimer = 0;
}

// Reading Inputs
void StageManager::upgrade_bag() {
	currentBagLevel++;

	partsPerSecond = (int)std::round(partsPerSecond * 1.1f);
	bagCap += (int)std::round(baseBagCap * 0.5f);
	bagUpgradeCost += (int)std::round(baseBagUpgradeCost * 0.25f);

	ui.bagUpgradeCostText.setString(std::format("${}", bagUpgradeCost));
	ui.partsCountText.setString(std::format("#{}/{}", parts, bagCap));

}
void StageManager::pause() {
	ui.paused = !ui.paused;
	cam.change_lock(ui.paused);
}
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
void StageManager::read_button_inputs(Key key) {
	if (key == Key::F)
		try_fire_cannon();
	else if (key == Key::B && try_buy_upgrade_bag())
		upgrade_bag();
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
void StageManager::handle_events(sf::Event event) {
	if (auto keyEvent = event.getIf<sf::Event::KeyPressed>()) {
		if (keyEvent->code == Key::Escape) {
			pause();
			return;
		}

		if (paused()) return;
		else if (read_spawn_inputs(keyEvent->code)) return;
		else if (read_lane_switch_inputs(keyEvent->code)) return;
		else read_button_inputs(keyEvent->code);
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
void StageManager::update_unit_ticks(float deltaTime) {
	//std::cout << "new update_unit_ticks() call" << std::endl;
	for (auto& lane : stage.lanes) {
		if (lane.trap) lane.trap->tick(deltaTime, stageRecorder);
		for (auto it = lane.enemyUnits.begin(); it != lane.enemyUnits.end();) {
			if (it->dead()) {
				handle_enemy_unit_death(*it);
				it = lane.enemyUnits.erase(it);
			}
			else {
				it->tick(deltaTime);
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
				it->tick(deltaTime);
				++it;
			}
		}
	}
}
void StageManager::update_ptr_ticks(float deltaTime) {
	for (auto it = stage.surges.begin(); it != stage.surges.end();) {
		if ((*it)->readyForRemoval) it = stage.surges.erase(it);
		else {
			(*it)->tick(deltaTime, stage);
			++it;
		}
	}
	for (auto it = stage.actionObjects.begin(); it != stage.actionObjects.end();) {
		if ((*it)->readyForRemoval) it = stage.actionObjects.erase(it);
		else {
			(*it)->tick(deltaTime);
			++it;
		}
	}
}
void StageManager::update_base_ticks(float deltaTime) {
	stage.playerBase.tick(stage,deltaTime);
	stage.enemyBase.tick(stage, deltaTime);
}

void StageManager::draw(sf::RenderWindow& window) {
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

	stage.enemyBase.draw(window);
	stage.playerBase.draw(window);

	for (auto it = stage.surges.begin(); it != stage.surges.end();) {
		window.draw((*it)->sprite);
		++it;
	}
	for (auto it = stage.actionObjects.begin(); it != stage.actionObjects.end();) {
		window.draw((*it)->sprite);
		++it;
	}
}
void StageManager::update_game_ticks(float deltaTime) {
	timeSinceStart += deltaTime;

	spawn_enemies(deltaTime);
	update_unit_ticks(deltaTime); // This is where the erase is called
	update_ptr_ticks(deltaTime);
	update_base_ticks(deltaTime);
	process_move_requests(); // this is where the other erase is called
	increment_parts_and_notify(deltaTime);
}