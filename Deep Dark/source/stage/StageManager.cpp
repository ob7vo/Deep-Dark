#include "pch.h"
#include "StageManager.h"
#include "Surge.h"

using json = nlohmann::json;

float random_float(float min, float max) {
	if (min == max) return min;

	static std::minstd_rand gen(std::random_device{}());
	std::uniform_real_distribution<float> dis(min, max);
	return dis(gen);
}

void StageManager::create_stage(const json& stageJson, int stageSet) 
{
	int lanes = stageJson["lane_count"];

	challenges.clear();
	stageRecorder = StageRecord(lanes);
	stage = Stage(stageJson, stageSet, &stageRecorder);
	wallet = {};

	create_challenges(stageJson);
	set_texts();
}
void StageManager::create_challenges(const json& stageJson) {
	size_t challengeCount = stageJson["challenges"].size();
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
}
void StageManager::set_texts() {
	ui.partsCountText.setString(std::format("$0/{}", wallet.partsCap));
	ui.bagUpgradeCostText.setString(std::format("${}", wallet.upgradeCost));
	ui.clearedChallengesText.setString(
		std::format("Challenges Cleared: 0/{}", challenges.size()));
	ui.clearedChallengesText.setCharacterSize(16);
	ui.clearedChallengesText.setFillColor(sf::Color::Yellow);
}
void StageManager::unload() {
	challenges.clear();
	stageRecorder = {};
	stage = {};
	wallet = {};
}

// Reading Inputs
void StageManager::upgrade_bag() {
	wallet.curLevel++;

	wallet.partsPerSecond = (int)std::round((float)wallet.partsPerSecond * 1.1f);
	wallet.partsCap += (int)std::round((float)wallet.basePartsCap * 0.5f);
	wallet.upgradeCost += (int)std::round((float)wallet.baseUpgradeCost * 0.25f);

	ui.bagUpgradeCostText.setString(std::format("${}", wallet.upgradeCost));
	ui.partsCountText.setString(std::format("#{}/{}", wallet.parts, wallet.partsCap));

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
	else if (key == Key::B && wallet.try_buy_upgrade_bag(stageRecorder))
		upgrade_bag();
}
bool StageManager::read_spawn_inputs(Key key) {
	for (int i = 0; i < loadout.filledSlots; i++) {
		LoadoutSlot& slot = loadout.slots[i];

		if ((key == numberKeys[i] || slot.autoDeploy) && slot.cooldown <= 0) {
			selectedLane %= stage.laneCount;

			if (!wallet.try_spend_parts(slot.unitStats.parts, stageRecorder)) return false;
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
	if (!unit.stats->try_proc_augment(DEATH_SURGE)) return;
	const Augment& augment = unit.stats->get_augment(DEATH_SURGE);

	AugmentType surgeType = FIRE_WALL; 
	if (unit.stats->quickAugMask & SHOCK_WAVE) surgeType = SHOCK_WAVE;
	else if (unit.stats->quickAugMask & ORBITAL_STRIKE) surgeType = ORBITAL_STRIKE; 

	float distance = augment.value;
	Augment newSurge = Augment::surge(surgeType, distance, augment.surgeLevel, 0, unit.combat.hitIndex);
	stage.create_surge(unit, newSurge);
}
void StageManager::create_drop_box(int laneInd, const UnitStats* stats, UnitAniMap* aniMap) {
	if (!stats->has_augment(DROP_BOX)) return;
	const Lane& lane = stage.lanes[laneInd];

	float percentage = stats->get_augment(DROP_BOX).value;
	float spawnPoint = lane.playerXPos + (lane.enemyXPos - lane.playerXPos) * percentage;

	sf::Vector2f spawnPos = { spawnPoint, lane.yPos };
	stage.entities.emplace_back(
		std::make_unique<UnitSpawner>(stats, aniMap, spawnPos, laneInd));
}
void StageManager::try_create_cloner(Unit& unit) {
	if (!unit.has_augment(CLONE) || unit.status.statusFlags & CODE_BREAKER) return;

	stage.entities.emplace_back(std::make_unique<UnitSpawner>
		(unit.stats, unit.anim.get_ani_map(), unit.get_pos(), unit.get_lane()));
}

void StageManager::collect_parts(Unit& unit) {
	int p = unit.stats->parts;
	if (unit.status.statusFlags & PLUNDER) p *= 2;
	wallet.gain_parts(p, stageRecorder);
}
void StageManager::increment_parts_and_notify(float deltaTime) {
	oneSecondTimer += deltaTime;
	while (oneSecondTimer >= 1.0f) {
		wallet.gain_parts(wallet.partsPerSecond, stageRecorder);
		notify_challenges();
		oneSecondTimer -= 1.0f;
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

void StageManager::spawn_enemies() {
	for (size_t i = stage.enemySpawners.size(); i--;) {
		auto& data = stage.enemySpawners[i];

		if (data.can_force_spawn(timeSinceStart)) {
			stage.create_unit(data.forcedSpawnTimes[0].second, &data.enemyStats, &data.aniArr);
			data.forcedSpawnTimes.erase(data.forcedSpawnTimes.begin());
		}

		int curIndex = data.currentSpawnIndex;
		if (curIndex >= data.totalSpawns || curIndex < 0) continue;

		if (timeSinceStart > data.nextSpawnTime) {
			data.nextSpawnTime += random_float(data.spawnDelays.first, data.spawnDelays.second);

			int laneIndex = data.laneSpawnIndexes[curIndex];
			stage.create_unit(laneIndex, &data.enemyStats, &data.aniArr);

			if (++data.currentSpawnIndex >= data.totalSpawns && data.infinite)
				data.currentSpawnIndex = 0;
		}
	}
}
void StageManager::process_move_requests() {
	for (size_t i = stage.moveRequests.size(); i--;) {
		auto const& request = stage.moveRequests[i];
		std::cout << "proccessing move request, ID = " << request.unitId << std::endl;

		auto& sourceVec = stage.get_source_vector(request.currentLane, request.team);
		auto unit = std::find_if(sourceVec.begin(), sourceVec.end(),
			[id = request.unitId](const Unit& u) { return u.id == id; });

		if (unit == sourceVec.end() || request.currentLane == request.newLane) {
			stage.moveRequests.pop_back();
			std::cout << "ignoring move request" << std::endl;
			continue;
		}

		unit->movement.cancel_tween();

		auto& newVec = stage.get_source_vector(request.newLane, unit->stats->team);
		Unit& movedUnit = newVec.emplace_back(std::move(*unit));
		sourceVec.erase(unit);

		request.move_unit_by_request(movedUnit, stage);

		stage.moveRequests.pop_back();
	}
}

void StageManager::update_unit(float deltaTime) {
	for (auto& lane : stage.lanes) {
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
				it = lane.playerUnits.erase(it);
			}
			else {
				it->tick(deltaTime);
				++it;
			}
		}
	}
}
void StageManager::update_projectiles(float deltaTime) {
	for (auto it = stage.projectiles.begin(); it != stage.projectiles.end();) {
		if (it->readyForRemoval) {
			it = stage.projectiles.erase(it);
			std::cout << "removed projectile" << std::endl;
		}
		else {
			Lane& closestLane = stage.get_closest_lane(it->y_pos());
			it->tick(closestLane, deltaTime);
			++it;
		}
	}
}
void StageManager::update_entities(float dt) {
	for (auto it = stage.surges.begin(); it != stage.surges.end();) {
		if ((*it)->readyForRemoval) it = stage.surges.erase(it);
		else {
			(*it)->tick(dt, stage);
			++it;
		}
	}

	for (auto const& entity : stage.entities) 
		entity->tick(stage, dt);
	for (auto& tp : stage.teleporters)
		tp.tick(stage, dt);
	for (auto& trap : stage.traps)
		trap.tick(stage, dt);

	std::erase_if(stage.entities, [](const auto& e) {
		return e->readyForRemoval;
		});
}
void StageManager::update_base(float deltaTime) {
	stage.playerBase.tick(stage, deltaTime);

	stage.enemyBase.tick(stage, deltaTime);
	if (stage.enemyBase.tookDmgThisFrame)
		stage.break_spawner_thresholds(timeSinceStart);
}
void StageManager::update_hitbox_visualizers(float deltaTime) {
	for (size_t i = stage.hitboxes.size(); i--;) {
		stage.hitboxes[i].second -= deltaTime;

		if (stage.hitboxes[i].second <= 0) {
			stage.hitboxes[i] = std::move(stage.hitboxes.back());
			stage.hitboxes.pop_back();
		}
	}
}

void StageManager::draw(sf::RenderWindow& window) {
	for (auto const& lane : stage.lanes) {
		lane.draw(window);
		for (auto const& unit : lane.enemyUnits)
			unit.anim.draw(window);
		for (auto const& unit : lane.playerUnits)
			unit.anim.draw(window);		
	}
	for (auto& proj : stage.projectiles)
		window.draw(proj.get_sprite());

	stage.enemyBase.draw(window);
	stage.playerBase.draw(window);

	for (auto it = stage.surges.begin(); it != stage.surges.end();) {
		(*it)->draw(window);
		++it;
	}
	for (auto const& entity : stage.entities)
		window.draw(entity->sprite);
	for (auto const& tp : stage.teleporters)
		tp.draw(window);
	for (auto const& trap : stage.traps)
		window.draw(trap.sprite);

	for (auto const& [effect, pos] : stage.effectSpritePositions) {
		int i = Augment::get_bit_position(effect);

		sf::Sprite& sprite = effectSprites[i];
		sprite.setPosition(pos);
		window.draw(sprite);
	}
	for (auto const& [hitbox, time]:stage.hitboxes)
		window.draw(hitbox);
}
void StageManager::update_game_ticks(float deltaTime) {
	timeSinceStart += deltaTime;
	stage.effectSpritePositions.clear();

	spawn_enemies();

	update_unit(deltaTime); // This is where the erase is called
	update_projectiles(deltaTime);
	update_entities(deltaTime);
	update_base(deltaTime);
	update_hitbox_visualizers(deltaTime);

	process_move_requests(); // this is where the other erase is called
	increment_parts_and_notify(deltaTime);
}
