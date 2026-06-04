#include "pch.h"
#include "Spawners.h"
#include "Stage.h"
#include "EntityTextures.h"
#include "UnitConfig.h"
#include "Utils.h"

#pragma region Constructors
EnemySpawner::EnemySpawner(const nlohmann::json& spawnerData) 
	: unitMagnification(spawnerData["magnification"].get<float>())
	, spawnsABoss(spawnerData.value("is_a_boss", false))
	, firstSpawnTime(spawnerData["first_spawn_time"].get<float>())
	, nextSpawnTime(INACTIVE_SPAWNER_TIMER)
	, percentThreshold(spawnerData.value("percent_threshold", 101.0f))
	, spawnDelays(spawnerData["spawn_delays"].get<std::pair<float, float>>())
	, forcedSpawnTimes(spawnerData.value("forced_spawn_times", std::vector<std::pair<float, int>>{}))
	, currentSpawnIndex(INACTIVE_SPAWN_INDEX)
	, totalSpawns(spawnerData["total_spawns"].get<int>())
	, infinite(spawnerData.contains("infinite"))
	, laneSpawnIndexes(spawnerData["lane_indexes"].get<std::vector<int>>())
{}
void EnemySpawner::create_unit_data(const nlohmann::json& spawnerData) {
	const nlohmann::json unitJson = UnitConfig::createUnitJson(spawnerData["unit_id"].get<int>());

	AnimationClip::setup_unit_animation_map(unitJson, aniMap, unitTextures);
	enemyStats = UnitStats::create_enemy(unitJson, spawnerData["magnification"].get<float>());
} 

UnitSpawner::UnitSpawner(const UnitStats* stats, UnitAniMap* aniMap, sf::Vector2f pos, int lane) :
	StageEntity(pos, lane), stats(stats), aniMap(aniMap) {
	create_animation();
}
SurgeSpawner::SurgeSpawner(const UnitStats* stats, const Augment& surge, sf::Vector2f pos, int lane) :
	StageEntity(pos, lane), stats(stats), surgeAugment(surge) {
	create_animation();
}
#pragma endregion

void EnemySpawner::forcefully_spawn_an_enemy(Stage& stage) {
	stage.create_unit(forcedSpawnTimes[0].second, &enemyStats, &aniMap);
	forcedSpawnTimes.erase(forcedSpawnTimes.begin());
}
void EnemySpawner::spawn_an_enemy(Stage& stage) {
	nextSpawnTime += Random::r_float(spawnDelays.first, spawnDelays.second);

	stage.create_unit(laneSpawnIndexes[currentSpawnIndex], &enemyStats, &aniMap);

	currentSpawnIndex++;
	if (currentSpawnIndex >= totalSpawns && infinite)
		currentSpawnIndex = 0;
}
void EnemySpawner::unleash_boss_shockwave(Stage& stage) const {
	for (const auto& lane : stage.lanes) {
		for (const auto& index : lane.getOpponentUnits(UnitConfig::ENEMY_TEAM)){
			auto& playerUnit = stage.getUnit(index);
			if (!playerUnit.anim.invincible())
				playerUnit.movement.knockback(UnitConfig::BOSS_SHOCKWAVE_KB_FORCE);
		}
	}
}

void UnitSpawner::action(Stage& stage) {
	if (stats->has_augment(AugmentType::CLONE)) stage.try_revive_unit(this);
	else {
		Unit* unit = stage.create_unit(laneIdx, stats, aniMap);
		unit->movement.pos.x = sprite.getPosition().x;
	}
}
void SurgeSpawner::action(Stage& stage) {
	sf::Vector2f surgePos = sprite.getPosition();

	if (surgeAugment.augType != AugmentType::SHOCK_WAVE)
		surgePos.x += surgeAugment.data.surge.spawnDistance * static_cast<float>(stats->team);

	stage.create_surge(stats, laneIdx, surgeAugment.data.surge.level, surgePos, surgeAugment.augType);
}

void UnitSpawner::create_animation() {
	sf::Texture* pTexture = stats->has_augment(AugmentType::CLONE) ?
		&Textures::Entity::t_cloner : &Textures::Entity::t_dropbox;

	sf::Vector2i cellSize = { 48,40 };
	sf::Vector2f origin = { 24,40 };

	int frames = 21;
	float rate = 0.1f;

	animClip = AnimationClip(pTexture, frames, rate, cellSize, origin, {{14, AnimationEvent::TRIGGER}});
	animPlayer.start(&animClip, sprite);
}
void SurgeSpawner::create_animation() {
	sf::Vector2i cellSize = { 48,40 };
	sf::Vector2f origin = { 24,48 };

	int frames = 21;
	float rate = 0.09f;

	animClip = AnimationClip(&Textures::Entity::t_surgeSpawner, 
		frames, rate, cellSize, origin, {{14, AnimationEvent::TRIGGER}});
	animPlayer.start(&animClip, sprite);
}