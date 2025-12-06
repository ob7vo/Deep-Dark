#include "pch.h"
#include "Spawners.h"
#include "Stage.h"

#pragma region Constructors
EnemySpawner::EnemySpawner(const nlohmann::json& spawnerData, Stage& stage) {
	currentSpawnIndex = -1;
	totalSpawns = spawnerData["total_spawns"];

	float magnification = spawnerData["magnification"];

	float firstSpawnTime = spawnerData["first_spawn_time"];
	spawnDelays = spawnerData["spawn_delays"].get<std::pair<float, float>>(); // min-max pair.

	std::vector<int> laneIndexes = spawnerData["lane_indexes"];
	laneSpawnIndexes = laneIndexes;

	infinite = spawnerData.contains("infinite");
	percentThreshold = spawnerData.value("percent_threshold", 101.0f);

	if (spawnerData.contains("forced_spawn_times"))
		for (auto& forcedTime : spawnerData["forced_spawn_times"])
			forcedSpawnTimes.emplace_back((float)forcedTime[0], (int)forcedTime[1]);
	nextSpawnTime = firstSpawnTime + INACTIVE_SPAWNER;

	int id = spawnerData["unit_id"];

	const nlohmann::json unitFile = UnitData::createUnitJson(id);
	Animation::setup_unit_animation_map(unitFile, aniArr);
	enemyStats = UnitStats::enemy(unitFile, magnification);

	for (const auto& aug : enemyStats.augments)
		if (aug.augType & PROJECTILE)
			stage.projConfigs[id] = ProjectileConfig(id, magnification);
}

UnitSpawner::UnitSpawner(const UnitStats* stats, UnitAniMap* aniMap, sf::Vector2f pos, int lane) :
	StageEntity(pos, lane), stats(stats), aniMap(aniMap) {}
SurgeSpawner::SurgeSpawner(const UnitStats* stats, const Augment& surge, sf::Vector2f pos, int lane) :
	StageEntity(pos, lane), stats(stats), surge(surge) {}
#pragma endregion

void UnitSpawner::action(Stage& stage) {
	if (stats->has_augment(CLONE)) stage.try_revive_unit(this);
	else {
		Unit* unit = stage.create_unit(laneInd, stats, aniMap);
		unit->movement.pos.x = sprite.getPosition().x;
	}
}
void SurgeSpawner::action(Stage& stage) {
	sf::Vector2f surgePos = sprite.getPosition();

	if (surge.augType != AugmentType::SHOCK_WAVE)
		surgePos.x += surge.value * static_cast<float>(stats->team);

	stage.create_surge(stats, laneInd, surge.surgeLevel, surgePos, surge.augType);
}

void UnitSpawner::create_animation() {
	std::string path = stats->has_augment(CLONE) ? "sprites/action_objs/cloner.png" :
		"sprites/action_objs/drop_box.png";
	sf::Vector2i cellSize = { 48,40 };
	sf::Vector2f origin = { 24,40 };

	int frames = 21;
	float rate = 0.1f;
	anim = Animation(path, frames, rate, cellSize, origin, {{14, TRIGGER}});

	anim.reset(sprite);
}
void SurgeSpawner::create_animation() {
	std::string path = "sprites/action_objs/surge_spawner.png";
	sf::Vector2i cellSize = { 48,40 };
	sf::Vector2f origin = { 24,48 };

	int frames = 21;
	float rate = 0.09f;
	anim = Animation(path, frames, rate, cellSize, origin, {{14, TRIGGER}});
	anim.reset(sprite);
}