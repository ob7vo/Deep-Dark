#include "StageManager.h"
#include <ranges>
#include <algorithm>

using json = nlohmann::json;

float random_float(float min, float max) {
	if (min == max) return min;

	static std::minstd_rand gen(std::random_device{}());
	std::uniform_real_distribution<float> dis(min, max);
	return dis(gen);
}

StageManager::StageManager(const json& stageJson, std::vector<std::string>& slotsJsonPaths) :
	stage(stageJson, &selectedLane), loadout(slotsJsonPaths), selectedLane(0){
}
void StageManager::read_lane_switch_inputs(Key key) {
	if (key == Key::Up)
		selectedLane = (selectedLane + 1) % stage.laneCount;
	else if (key == Key::Down)
		selectedLane = (selectedLane - 1 + stage.laneCount) % stage.laneCount;
}
void StageManager::read_spawn_inputs(Key key) {
	for (int i = 0; i < 10; i++) {
		if (!loadout.filled_slot(i)) return;

		if (key == numberKeys[i]) {
			selectedLane %= stage.laneCount;
			Slot& slot = loadout.slots[i];
			stage.create_unit(selectedLane, &slot.unitStats, &slot.aniMap);
			return;
		}
	}
}
void StageManager::read_base_fire_input(Key key) {
	if (key == Key::F) {
		stage.playerBase.fire_cannon();
	}
}
void StageManager::handle_events(sf::Event event) {
	if (auto keyEvent = event.getIf<sf::Event::KeyPressed>()) {
		read_lane_switch_inputs(keyEvent->code);
		read_spawn_inputs(keyEvent->code);
		read_base_fire_input(keyEvent->code);
	}
}
void StageManager::try_spawn_death_surge(Unit& unit) {
	if (!unit.trigger_augment(unit.stats, DEATH_SURGE)) return;
	const Augment& augment = unit.stats->get_augment(DEATH_SURGE);

	AugmentType surgeType = FIRE_WALL; 
	if (unit.stats->quickAugMask & SHOCK_WAVE) surgeType = SHOCK_WAVE;
	else if (unit.stats->quickAugMask & ORBITAL_STRIKE) surgeType = ORBITAL_STRIKE; 

	float distance = augment.value;
	Augment newSurge(surgeType, distance, 0.f, augment.surgeLevel);
	stage.create_surge(unit, newSurge);
}
void StageManager::try_revive_unit(Unit& unit) {
	// if they dont have the clone ability, or have be inflicted with code breaker, return
	if (!unit.has_augment(CLONE) || unit.statuses & CODE_BREAKER) return;

	Unit* newUnit = stage.create_unit(unit.currentLane, unit.stats, unit.get_ani_array());
	if (newUnit) {
		int maxHp = unit.stats->maxHp;
		int newHp = maxHp * unit.stats->get_augment(CLONE).value;
		newUnit->hp = newHp;
		for (int i = 1; i <= unit.stats->knockbacks; i++) {
			int threshold = maxHp - (maxHp * i / unit.stats->knockbacks);
			if (newHp <= threshold)
				newUnit->kbIndex = i + 1;
		}
		std::cout << "cloned unit, kbIndex = " << newUnit->kbIndex << std::endl;

		newUnit->statuses |= CODE_BREAKER;
		newUnit->pos = unit.pos;
		newUnit->start_animation(UnitAnimationState::MOVING);
	}
}
void StageManager::collect_parts(Unit& unit) {
	int p = unit.stats->parts;
	if (unit.statuses & PLUNDER) p *= 2;
	parts = std::clamp(parts + p, 0, walletCap);
}
void StageManager::handle_enemy_unit_death(Unit& unit) {
	try_spawn_death_surge(unit);
	try_revive_unit(unit);
	collect_parts(unit);
}
void StageManager::handle_player_unit_death(Unit& unit) {
	try_spawn_death_surge(unit);
	try_revive_unit(unit);
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
			data.nextSpawnTime += random_float(data.minSpawnDelay, data.maxSpawnDelay);
			int laneIndex = data.laneToSpawn[data.currentSpawnIndex];
			stage.create_unit(laneIndex, &data.enemyStats, &data.aniArr);

			data.currentSpawnIndex++;
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
			continue;
		}
	//	else if (true) {
//
	//	}
		unit->cancel_tweens();

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
		auto dead_begin = std::ranges::remove_if(lane.playerUnits, [&window, &deltaTime](auto& unit) {
			unit.tick(window, deltaTime);

			if (unit.dead()) {
				unit.cancel_tweens();
				return true;
			}
			else return false;
			});
	    lane.playerUnits.erase(dead_begin.begin(), dead_begin.end());
		//lane.playerUnits.erase(dead_begin, lane.playerUnits.end());

		//std::erase_if(lane.playerUnits, [&window, &deltaTime](auto& unit) {
			//unit.tick(window, deltaTime);

			//return unit.dead();
		//});

		//lane.playerUnits.erase(dead_begin, lane.playerUnits.end());
		for (auto it = lane.enemyUnits.begin(); it != lane.enemyUnits.end();) {
			std::cout << "enemy tick" << std::endl;
			if (it->dead()) {
				handle_enemy_unit_death(*it);
				it = lane.enemyUnits.erase(it);
			}
			else {
				it->tick(window, deltaTime);
				++it;
			}
		}
		/*
		for (auto it = lane.playerUnits.begin(); it != lane.playerUnits.end();) {
			if (it->dead()) {
				//handle_player_unit_death(*it);
				std::cout << "Player Unit #" << it->id << " on Lane " << it->currentLane <<
					"is DEAD, erasing it from the vector" << std::endl;
				it = lane.playerUnits.erase(it);
				int dummy = 0;
			}
			else {
				//std::cout << "Unit #" << it->id << " is NOT DEAD, hp: " << it->hp <<
			//		" - AnimationState: " << (int)it->get_state() << std::endl;
				it->tick(window, deltaTime);
				++it;
			}
		}
		*/
	}
}
void StageManager::update_surge_ticks(sf::RenderWindow& window, float deltaTime) {
	for (auto it = stage.surges.begin(); it != stage.surges.end();) {
		if ((*it)->readyForRemoval) it = stage.surges.erase(it);
		else {
			(*it)->tick(window, deltaTime, stage);
			++it;
		}
	}
}
void StageManager::update_base_ticks(sf::RenderWindow& window, float deltaTime) {
	stage.playerBase.tick(stage, window, deltaTime);
	stage.enemyBase.tick(stage, window, deltaTime);
}
void StageManager::only_draw(sf::RenderWindow& window) {
	for (auto& lane : stage.lanes) {
		lane.draw(window);
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
void StageManager::update_game_ticks(sf::RenderWindow& window, float deltaTime) {
	//spawn_enemies(deltaTime);
	update_unit_ticks(window, deltaTime); // This is where the erase is called
	//update_surge_ticks(window, deltaTime);
	//update_base_ticks(window, deltaTime);
	//process_move_requests(); // this is where the other erase is called
}