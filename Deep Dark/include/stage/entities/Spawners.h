 #pragma once
#include <StageEntity.h>
#include <UnitData.h>
#include "Augment.h"

struct UnitSpawner : public StageEntity {
	const UnitStats* stats;
	UnitAniMap* aniMap;

	UnitSpawner(const UnitStats* stats, UnitAniMap* aniMap, sf::Vector2f pos, int lane);
	~UnitSpawner() override = default;

	void action(Stage& stage) override;
	void create_animation();
};

struct SurgeSpawner : public StageEntity {
	const UnitStats* stats;
	const Augment surge;

	SurgeSpawner(const UnitStats* stats, const Augment& surge, sf::Vector2f pos, int lane);
	~SurgeSpawner() override = default;

	void action(Stage& stage) override;
	void create_animation();
};

struct EnemySpawner {
	UnitStats enemyStats;
	UnitAniMap aniArr;

	float nextSpawnTime = 10000.f;
	float percentThreshold = 101.f;
	std::pair<float, float> spawnDelays;
	///<summary>First is the EXACT time an enemy will spawn, Second is the lane </summary>
	std::vector<std::pair<float, int>> forcedSpawnTimes = {};

	int currentSpawnIndex = -1;
	int totalSpawns = 0;
	bool infinite = false;

	std::vector<int> laneSpawnIndexes = {};

	EnemySpawner(const nlohmann::json& spawnerData, Stage& stage);

	inline bool can_force_spawn(float time) { return !forcedSpawnTimes.empty() && time > forcedSpawnTimes[0].first; }
};
