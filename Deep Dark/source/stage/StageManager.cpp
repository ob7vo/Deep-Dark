#include "pch.h"
#include "StageManager.h"
#include "Surge.h"
#include "EffectTextures.h"
#include "UITextures.h"

using json = nlohmann::json;
using namespace Textures::Effects;

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

void StageManager::create_stage(const json& stageJson, int stageSet) {
	const json& stageSetJson = stageJson["sets"][stageSet];
	auto lanes = static_cast<int>(stageSetJson["lanes"].size());

	challenges.clear();
	stageRecorder = StageRecord(lanes);
	stage = Stage(stageSetJson, &stageRecorder);
	wallet = {};

	create_challenges(stageSetJson);
	set_texts();
}
void StageManager::create_challenges(const json& stageSetJson) {
	size_t challengeCount = stageSetJson["challenges"].size();
	challenges.reserve(challengeCount);

	for (auto& chalJson : stageSetJson["challenges"]) {
		StageChallenge& chal = challenges.emplace_back(chalJson);
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

// Buttons
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

			if (slot.unitStats.has_augment(AugmentType::DROP_BOX)) 
				try_create_drop_box(selectedLane, &slot.unitStats, &slot.aniMap);
			else
				stage.create_unit(selectedLane, &slot.unitStats, &slot.aniMap);

			return true;
		}
	}
	return false;
}
void StageManager::handle_events(sf::Event event) {
	// If the stage set has ended at all, don't read inputs
	if (stage.victoriousTeam != 0) return;

	if (auto keyEvent = event.getIf<sf::Event::KeyPressed>()) {
		if (keyEvent->code == Key::Escape) {
			pause();
			return;
		}

		// The functions return bools to have early returns
		if (paused()) return;
		else if (read_spawn_inputs(keyEvent->code)) return;
		else if (read_lane_switch_inputs(keyEvent->code)) return;
		else read_button_inputs(keyEvent->code);
	}
}

// Creating Entities
bool StageManager::try_spawn_death_surge(const Unit& unit) {
	// Will proc if it has the augment and rolls the chance
	if (!unit.stats->try_proc_augment(AugmentType::DEATH_SURGE)) return false;

	const Augment& augment = *unit.stats->get_augment(AugmentType::DEATH_SURGE);
	float distance = augment.value;

	Augment newSurge = Augment::surge(AugmentType::FIRE_WALL, distance, augment.surgeLevel, 0, unit.combat.hitIndex);
	stage.create_surge(unit, newSurge);

	return true;
}
bool StageManager::try_create_drop_box(int laneInd, const UnitStats* stats, UnitAniMap* aniMap) {
	if (!stats->has_augment(AugmentType::DROP_BOX)) return false;

	const Lane& lane = stage.lanes[laneInd];

	float percentage = stats->get_augment(AugmentType::DROP_BOX)->value;
	float spawnPoint = lane.playerSpawnPoint + (lane.enemySpawnPoint - lane.playerSpawnPoint) * percentage;

	sf::Vector2f spawnPos = { spawnPoint, lane.yPos };
	stage.entities.emplace_back(
		std::make_unique<UnitSpawner>(stats, aniMap, spawnPos, laneInd));

	return true;
}
bool StageManager::try_create_cloner(const Unit& unit) {
	if (!unit.stats->has_augment(AugmentType::CLONE) || has(unit.status.statusFlags & AugmentType::CODE_BREAKER)) 
		return false;

	stage.entities.emplace_back(std::make_unique<UnitSpawner>
		(unit.stats, unit.anim.get_ani_map(), unit.get_pos(), unit.get_lane()));

	return true;
}

// Parts
void StageManager::collect_parts(const Unit& unit) {
	int partsToGain = unit.stats->parts;
	if (has(unit.status.statusFlags & AugmentType::PLUNDER)) partsToGain *= 2;

	wallet.gain_parts(partsToGain, stageRecorder);
}
void StageManager::increment_parts_and_notify(float deltaTime) {
	oneSecondTimer += deltaTime;

	while (oneSecondTimer >= 1.0f) {
		wallet.gain_parts(wallet.partsPerSecond, stageRecorder);
		notify_challenges();

		oneSecondTimer -= 1.0f;
	}
}

// Handling Unit Death
void StageManager::handle_death_augment(const Unit& unit) {
	if (has(unit.causeOfDeath, DeathCause::FALLING) ||
		has(unit.causeOfDeath, DeathCause::BASE_WAS_DESTROYED))
		return;

	try_spawn_death_surge(unit);
	try_create_cloner(unit);
}
void StageManager::handle_enemy_unit_death(const Unit& unit) {
	handle_death_augment(unit);

	collect_parts(unit);
}
void StageManager::handle_player_unit_death(const Unit& unit) {
	handle_death_augment(unit);
}

// Creating Enemies
void StageManager::spawn_enemies() {
	for (size_t i = stage.enemySpawners.size(); i--;) {
		auto& data = stage.enemySpawners[i];

		if (data.can_force_spawn(stage.timeSinceStart)) {
			stage.create_unit(data.forcedSpawnTimes[0].second, &data.enemyStats, &data.aniMap);
			data.forcedSpawnTimes.erase(data.forcedSpawnTimes.begin());
		}

		int curIndex = data.currentSpawnIndex;
		// If there are no more units to spawn or the spawn is inactive (curIndex < 0)
		if (curIndex >= data.totalSpawns || curIndex < 0) continue;

		// Spawn the Unit
		if (stage.timeSinceStart > data.nextSpawnTime) {
			data.nextSpawnTime += random_float(data.spawnDelays.first, data.spawnDelays.second);

			int laneIndex = data.laneSpawnIndexes[curIndex];
			stage.create_unit(laneIndex, &data.enemyStats, &data.aniMap);

			if (++data.currentSpawnIndex >= data.totalSpawns && data.infinite)
				data.currentSpawnIndex = 0;
		}
	}
}

/*
	for (size_t i = stage.moveRequests.size(); i > 0; --i) {
		auto const& request = stage.moveRequests[i];
		std::cout << "proccessing move request, ID = " << request.unitId << std::endl;

		auto& originalAllyUnits = stage.lanes[request.currentLane].getAllyUnits(request.team);
		auto unit = std::find_if(originalAllyUnits.begin(), originalAllyUnits.end(),
			[id = request.unitId](const Unit& u) { return u.id == id; });

		if (unit == originalAllyUnits.end() || request.currentLane == request.newLane) {
			stage.moveRequests.pop_back();
			std::cout << "ignoring move request" << std::endl;
			continue;
		}

		// Cancel Tweens to be safe.
		unit->movement.cancel_tween();

		auto& newLaneVector = stage.lanes[request.newLane].getAllyUnits(request.team);
		Unit& movedUnit = newLaneVector.emplace_back(std::move(*unit));
		originalAllyUnits.erase(unit);

		request.move_unit_by_request(movedUnit, stage);

		stage.moveRequests.pop_back();
	}
	*/
// Move Requests
void StageManager::process_all_move_requests() {
	while (!stage.moveRequests.empty()) {
		auto& moveRequest = stage.moveRequests.back();

		if (auto index = find_unit_to_move(moveRequest))
			process_move_request(moveRequest, *index);

		stage.moveRequests.pop_back();
	}
}
std::optional<size_t> StageManager::find_unit_to_move(const UnitMoveRequest& moveRequest) {
	auto& from = stage.lanes[moveRequest.currentLane].getAllyUnits(moveRequest.team);

	auto unit = std::find_if(from.begin(), from.end(),
		[id = moveRequest.unitId](const Unit& u) { return u.id == id; });

	if (unit == from.end()) {
		std::cout << "Unit was not found in vector." << std::endl;
		return std::nullopt;
	}
	else if (moveRequest.currentLane == moveRequest.newLane) {
		std::cout << "MoveRequest moves Unit to the same lane." << std::endl;
		return std::nullopt;
	}

	return std::distance(from.begin(), unit);
}
void StageManager::process_move_request(const UnitMoveRequest& moveRequest, size_t unitToMoveIndex)
{
	auto& from = stage.lanes[moveRequest.currentLane].getAllyUnits(moveRequest.team);
	auto& unit = from[unitToMoveIndex];

	unit.movement.cancel_tween();

	auto& to = stage.lanes[moveRequest.newLane].getAllyUnits(moveRequest.team);
	Unit& movedUnit = to.emplace_back(std::move(unit));

	from.erase(from.begin() + unitToMoveIndex);

	moveRequest.move_unit_by_request(movedUnit, stage);
}

// Updating GameObjects
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
		}
		else {
			it->tick(stage, deltaTime);
			++it;
		}
	}
}
void StageManager::update_entities(float dt) {
	// Ticking Unique Pointers
	for (auto const& surge : stage.surges)
		surge->tick(stage, dt);
	for (auto const& entity : stage.entities) 
		entity->tick(stage, dt);

	// Raw values (Won't delete)
	for (auto& tp : stage.teleporters)
		tp.tick(stage, dt);
	for (auto& trap : stage.traps)
		trap.tick(stage, dt);

	// Deleting Surges and Entities
	std::erase_if(stage.entities, [](const auto& entity) {
		return entity->readyForRemoval;
		});
	std::erase_if(stage.surges, [](const auto& surge) {
		return surge->readyForRemoval;
		}); 
}
void StageManager::update_base(float deltaTime) {
	stage.enemyBase.tick(stage, deltaTime);
	stage.playerBase.tick(stage, deltaTime);
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
		for (const auto& unit : lane.enemyUnits)
			unit.anim.draw(window);
		for (const auto& unit : lane.playerUnits)
			unit.anim.draw(window);		
	}

	for (auto& proj : stage.projectiles)
		window.draw(proj.get_sprite());

	stage.enemyBase.draw(window);
	stage.playerBase.draw(window);

	for (const auto& surge : stage.surges)
		surge->draw(window);
	for (const auto& entity : stage.entities)
		window.draw(entity->sprite);
	for (const auto& proj : stage.projectiles)
		proj.draw(window);
	for (const auto& tp : stage.teleporters)
		tp.draw(window);
	for (const auto& trap : stage.traps)
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
void StageManager::tick(float deltaTime) {
	stage.timeSinceStart += deltaTime;
	stage.effectSpritePositions.clear();

	// If the stage set is ongoing or enemies won, keeps spawning them in.
	if (!stage.reached_unit_capacity(ENEMY_TEAM) && stage.victoriousTeam != PLAYER_TEAM)
		spawn_enemies();

	update_unit(deltaTime); 
	update_projectiles(deltaTime);
	update_entities(deltaTime);
	update_base(deltaTime);
	update_hitbox_visualizers(deltaTime);

	process_all_move_requests();
	increment_parts_and_notify(deltaTime);
}

// Challenges
void StageManager::notify_challenges() {
	int clears = 0;

	for (auto& challenge : challenges) {
		challenge.notify(*this);
		if (challenge.cleared) clears++;
	}

	if (clears != clearedChallenges)
		update_challenges_text(clears);
}
void StageManager::update_challenges_text(int clears) {
	clearedChallenges = clears;

	if (clearedChallenges == challenges.size())
		ui.clearedChallengesText.setFillColor(sf::Color::Green);
	else
		ui.clearedChallengesText.setFillColor(sf::Color::Yellow);

	ui.clearedChallengesText.setString(std::format("Challenges Cleared: {}/{}",
		clearedChallenges, challenges.size()));
}

// Wallet
bool StageWallet::try_spend_parts(int partsToSpend, StageRecord& rec) {
	if (parts < partsToSpend) return false;

	parts = std::max(parts - partsToSpend, 0);
	rec.add_parts_spent(partsToSpend);
	return true;
}
void StageWallet::gain_parts(int partsToGain, StageRecord& rec) {
	parts = std::min(parts + partsToGain, partsCap);
	rec.add_parts_earned(partsToGain);
}