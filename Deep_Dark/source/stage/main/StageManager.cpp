#include "pch.h"
#include "StageManager.h"
#include "Surge.h"
#include "EffectTextures.h"
#include "UITextures.h"

using json = nlohmann::json;
using namespace Textures::Effects;
using enum StageStatus;

float random_float(float min, float max) {
	if (min == max) return min;

	static std::minstd_rand gen(std::random_device{}());
	std::uniform_real_distribution<float> dis(min, max);
	return dis(gen);
}
std::array<sf::Sprite, 8> StageManager::make_effSpriteArr() {
	std::array<sf::Sprite, 8> effArr = {
		sf::Sprite(t_statusIcons), sf::Sprite(t_statusIcons),
		sf::Sprite(t_statusIcons), sf::Sprite(t_statusIcons),
		sf::Sprite(t_statusIcons), sf::Sprite(t_statusIcons),
		sf::Sprite(t_statusIcons), sf::Sprite(t_statusIcons),
	};

	for (int i = 0; i < 8; i++)
		effArr[i].setTextureRect(Textures::UI::r_workshopStatsIcons[i]);

	return effArr;
}

void StageManager::create_stage(const json& stageJson, int stagePhase, bool inPracticeMode) {
	const json& stagePhaseJson = stageJson["phases"][stagePhase];
	auto lanes = static_cast<int>(stagePhaseJson["lanes"].size());

	// First phase, meaning there is no persistent data yet for the recorder and challenges
	if (stagePhase == 0 || inPracticeMode) {
		stageRecorder = StageRecord(lanes);

		challenges.clear();
		create_challenges(stageJson);
	}

	stageRecorder.currentStagePhase = stagePhase;
	stageRecorder.curPhaseElapsedTime = 0.f;

	stage = std::make_unique<Stage>(stagePhaseJson, &stageRecorder);
	wallet = {};
	
	stage->selectedLane = &selectedLane;
}
void StageManager::create_challenges(const json& stageJson) {
	size_t challengeCount = stageJson["challenges"].size();
	challenges.reserve(challengeCount);

	for (auto& chalJson : stageJson["challenges"]) {
		StageChallenge& chal = challenges.emplace_back(chalJson);
		chal.pTarget = chal.get_target_ptr(*this);
	}
}

#pragma region Creating Entities
bool StageManager::activate_units_self_destruct(const Unit& unit) {
	// Will proc if it has the augment and rolls the chance
	if (!unit.stats->has_augment(AugmentType::SELF_DESTRUCT)) return false;

	const Augment& aug = *unit.stats->get_augment(AugmentType::SELF_DESTRUCT);
	float range = aug.data.selfDestruct.explosionRange;
	int dmg = static_cast<int>(unit.stats->maxHp * aug.data.selfDestruct.hpPercentage);
	int maxLane = std::min(stage->laneCount - 1, unit.get_lane() + aug.data.selfDestruct.hitsAdjacentLanes);
	int minLane = std::max(0, unit.get_lane() - aug.data.selfDestruct.hitsAdjacentLanes);

	for (int i = minLane; i <= maxLane; i++) {
		const auto& enemyIndexes = stage->lanes[i].getOpponentUnits(unit.stats->team);

		for (const auto& index : enemyIndexes) {
			auto& enemyUnit = stage->getUnit(index);

			if (unit.found_valid_target(enemyUnit, -range, range)) {
				enemyUnit.status.apply_on_hit_status_effects(
					unit.stats->augments, ALL_HITS);
				enemyUnit.status.take_damage(dmg);
			}
		}
	}
	return true;
}
bool StageManager::try_create_drop_box(int laneIdx, const UnitStats* stats, UnitAnimMap* aniMap) {
	if (!stats->has_augment(AugmentType::DROP_BOX)) return false;

	const Lane& lane = stage->lanes[laneIdx];

	float percentage = stats->get_augment(AugmentType::DROP_BOX)->data.general.magnitude;
	float spawnPoint = lane.playerSpawnPoint + (lane.enemySpawnPoint - lane.playerSpawnPoint) * percentage;

	sf::Vector2f spawnPos = { spawnPoint, lane.yPos };
	stage->entities.emplace_back(std::make_unique<UnitSpawner>(stats, aniMap, spawnPos, laneIdx));

	return true;
}
bool StageManager::try_create_cloner(const Unit& unit) {
	if (!unit.stats->has_augment(AugmentType::CLONE) || has(unit.status.statusFlags & AugmentType::CODE_BREAKER)) 
		return false;

	stage->entities.emplace_back(std::make_unique<UnitSpawner>
		(unit.stats, unit.anim.get_ani_map(), unit.movement.pos, unit.get_lane()));

	return true;
}
#pragma endregion

void StageManager::call_one_second_updates(float deltaTime) {
	oneSecondTimer += deltaTime;

	while (oneSecondTimer >= 1.0f) {
		wallet.gain_parts(wallet.partsPerSecond, stageRecorder);
		notify_challenges();
		stage->unitAbilityObserver.notify(&TimedEvent::oneSecond);

		oneSecondTimer -= 1.0f;
	}
}

#pragma region Inputs
void StageManager::handle_events(sf::Event event) {
	// If the stage set has ended at all, don't read inputs
	if (has(stage->status, StageStatus::FINISHED)) return;

	if (auto keyEvent = event.getIf<sf::Event::KeyPressed>()) {
		if (keyEvent->code == Key::Escape) {
			ui.set_pause_state(!ui.paused);
			return;
		}

		// The functions return bools to have early returns
		if (ui.paused) return;
		else if (read_spawn_inputs(keyEvent->code)) return;
		else if (read_lane_switch_inputs(keyEvent->code)) return;
		else read_misc_button_inputs(keyEvent->code);
	}
}
bool StageManager::read_lane_switch_inputs(Key key) {
	if (key == Key::Up) {
		selectedLane = (selectedLane + 1) % stage->laneCount;
		return true;
	}
	else if (key == Key::Down) {
		selectedLane = (selectedLane - 1 + stage->laneCount) % stage->laneCount;
		return true;
	}

	return false;
}
void StageManager::read_misc_button_inputs(Key key) {
	if (key == Key::F)
		try_fire_cannon();
	else if (key == Key::B && wallet.can_upgrade_bag(stageRecorder)) {
		wallet.upgrade_bag();
		ui.update_texts();
	}
}
bool StageManager::read_spawn_inputs(Key key) {
	for (int i = 0; i < loadout.filledSlots; i++) {
		LoadoutSlot& slot = loadout.slots[i];

		if ((key == numberKeys[i] || slot.autoDeploy) && slot.cooldown <= 0) {
			selectedLane %= stage->laneCount;

			if (!wallet.try_spend_parts(slot.unitStats.parts, stageRecorder)) return false;
			slot.cooldown = slot.spawnTimer;

			if (slot.unitStats.has_augment(AugmentType::DROP_BOX))
				try_create_drop_box(selectedLane, &slot.unitStats, &slot.aniMap);
			else
				stage->create_unit(selectedLane, &slot.unitStats, &slot.aniMap);

			return true;
		}
	}
	return false;
}
#pragma endregion

void StageManager::handle_death_augment(const Unit& unit) {
	if (has(unit.causeOfDeath, DeathCause::FALLING) ||
		has(unit.causeOfDeath, DeathCause::BASE_WAS_DESTROYED))
		return;

	activate_units_self_destruct(unit);
	try_create_cloner(unit);
}
void StageManager::handle_unit_death(const Unit& unit, size_t poolIndex) {
	handle_death_augment(unit);

	stage->unitPool.freeUnit(poolIndex);

	if (unit.stats->team == UnitConfig::ENEMY_TEAM)
		wallet.collect_parts_from_unit(unit, stageRecorder);

	if (stage->unitAbilityObserver.has_salvage_unit())
		stage->unitAbilityObserver.registerDeadUnit(unit);
}

void StageManager::spawn_enemies() {
	for (size_t i = stage->enemySpawners.size(); i--;) {
		auto& spawner = stage->enemySpawners[i];

		if (spawner.can_force_spawn(stageRecorder.timeSinceStart)) {
			spawner.forcefully_spawn_an_enemy(*stage);
		}

		// If there are no more units to spawn or the spawn is inactive (curIndex < 0)
		if (spawner.currentSpawnIndex >= spawner.totalSpawns || spawner.currentSpawnIndex < 0) 
			continue;

		// Spawn the Unit
		if (stageRecorder.timeSinceStart > spawner.nextSpawnTime) {
			spawner.spawn_an_enemy(*stage);
		}
	}
}

void StageManager::process_all_move_requests() {
	while (!stage->laneTransferRequests.empty()) {
		const auto& moveRequest = stage->laneTransferRequests.back();
		
		// Get the index of the Unit in it's lane
		if (auto unitIndexInVector = moveRequest.find_unit_to_move(*stage))
			moveRequest.process(stage.get(), *unitIndexInVector);

		stage->laneTransferRequests.pop_back();
	}
}

// Updating GameObjects
void StageManager::update_units(std::vector<size_t>& unitIndexes, float deltaTime) {
	for (size_t i = 0; i < unitIndexes.size(); ) {
		Unit& unit = stage->getUnit(unitIndexes[i]);

		// Swap and pop
		if (unit.dead()) {
			handle_unit_death(unit, unitIndexes[i]);

			unitIndexes[i] = unitIndexes.back();
			unitIndexes.pop_back();
		}
		else {
			unit.tick(deltaTime);
			i++;
		}
	}
}
void StageManager::update_lanes(float deltaTime) {
	for (auto& lane : stage->lanes) {
		update_units(lane.enemyUnitIndexes, deltaTime);
		update_units(lane.playerUnitIndexes, deltaTime);
	}
}
void StageManager::update_projectiles(float deltaTime) {
	for (auto it = stage->projectiles.begin(); it != stage->projectiles.end();) {
		if (it->readyForRemoval) {
			it = stage->projectiles.erase(it);
		}
		else {
			it->tick(*stage, deltaTime);
			++it;
		}
	}
}
void StageManager::update_entities(float dt) {
	// Ticking Unique Pointers
	for (auto const& surge : stage->surges)
		surge->tick(*stage, dt);
	for (auto const& entity : stage->entities) 
		entity->tick(*stage, dt);

	// Raw values (Won't delete)
	for (auto& tp : stage->teleporters)
		tp.tick(*stage, dt);
	for (auto& trap : stage->traps)
		trap.tick(*stage, dt);

	// Deleting Surges and Entities
	std::erase_if(stage->entities, [](const auto& entity) {
		return entity->readyForRemoval;
		});
	std::erase_if(stage->surges, [](const auto& surge) {
		return surge->readyForRemoval;
		}); 
}
void StageManager::update_base(float deltaTime) {
	stage->enemyBase.tick(*stage, deltaTime);
	stage->playerBase.tick(*stage, deltaTime);
}
void StageManager::update_hitbox_visualizers(float deltaTime) {
	for (size_t i = stage->hitboxes.size(); i--;) {
		stage->hitboxes[i].second -= deltaTime;

		if (stage->hitboxes[i].second <= 0) {
			stage->hitboxes[i] = std::move(stage->hitboxes.back());
			stage->hitboxes.pop_back();
		}
	}
}

void StageManager::draw(sf::RenderWindow& window) {
	for (auto const& lane : stage->lanes) {
		lane.draw(window);
		for (const auto& index : lane.enemyUnitIndexes)
			stage->getUnit(index).anim.draw(window);
		for (const auto& index : lane.playerUnitIndexes)
			stage->getUnit(index).anim.draw(window);
	}

	for (auto& proj : stage->projectiles)
		window.draw(proj.get_sprite());

	stage->enemyBase.draw(window);
	stage->playerBase.draw(window);

	for (const auto& surge : stage->surges)
		surge->draw(window);
	for (const auto& entity : stage->entities)
		window.draw(entity->sprite);
	for (const auto& proj : stage->projectiles)
		proj.draw(window);
	for (const auto& tp : stage->teleporters)
		tp.draw(window);
	for (const auto& trap : stage->traps)
		window.draw(trap.sprite);

	for (auto const& [effect, pos] : stage->effectSpritePositions) {
		int i = Augment::get_bit_position(effect);

		sf::Sprite& sprite = effectSprites[i];
		sprite.setPosition(pos);
		window.draw(sprite);
	}
	for (auto const& [hitbox, time] : stage->hitboxes)
		window.draw(hitbox);
}
void StageManager::tick(float deltaTime) {
	stageRecorder.timeSinceStart += deltaTime;
	stageRecorder.curPhaseElapsedTime += deltaTime;
	stage->effectSpritePositions.clear();

	// If the stage set is ongoing or enemies won, keeps spawning them in.
	if (!stage->reached_unit_capacity(UnitConfig::ENEMY_TEAM) && has(stage->status, PLAYER_VICTORY))
		spawn_enemies();

	update_lanes(deltaTime); 
	update_projectiles(deltaTime);
	update_entities(deltaTime);
	update_base(deltaTime);
	update_hitbox_visualizers(deltaTime);

	process_all_move_requests();
	call_one_second_updates(deltaTime);
}

// Challenges
void StageManager::notify_challenges() {
	int clears = 0;

	for (auto& challenge : challenges) {
		challenge.notify(*this);
		if (challenge.cleared) clears++;
	}

	if (clears != clearedChallenges) {
		clearedChallenges = clears;
		ui.update_texts();
	}
}