 #pragma once
#include <StageEntity.h>
#include <UnitData.h>
#include <json_fwd.hpp>
#include "Augment.h"
#include <deque>

const int INACTIVE_SPAWN_INDEX = -1;
const float INACTIVE_SPAWNER_TIMER = 10000.f;

struct UnitSpawner : public StageEntity {
	const UnitStats* stats;
	UnitAniMap* aniMap;

	AnimationClip animClip;

	UnitSpawner(const UnitStats* stats, UnitAniMap* aniMap, sf::Vector2f pos, int lane);
	~UnitSpawner() override = default;

	void action(Stage& stage) override;
	void create_animation();
};

struct SurgeSpawner : public StageEntity {
	const UnitStats* stats;
	const Augment surge;

	AnimationClip animClip;

	SurgeSpawner(const UnitStats* stats, const Augment& surge, sf::Vector2f pos, int lane);
	~SurgeSpawner() override = default;

	void action(Stage& stage) override;
	void create_animation();
};

struct EnemySpawner {
	UnitStats enemyStats;
	std::deque<sf::Texture> unitTextures;
	UnitAniMap aniMap;

	float unitMagnification = 1.f;
	bool spawnsABoss = false;

	float firstSpawnTime = 0;
	float nextSpawnTime = 10000.f;
	/// <summary> When the Enemy Base's HP% drops below this, the spawner will activate. </summary>
	float percentThreshold = 101.f;
	std::pair<float, float> spawnDelays;
	///<summary>First is the EXACT time an enemy will spawn, Second is the lane index </summary>
	std::vector<std::pair<float, int>> forcedSpawnTimes = {};

	/// <summary>
	/// Index used for spawning the enemy on its correct lane and checking how many
	/// Units have sppawned in. If the spawner is inactive, the index will be -1
	/// </summary>
	int currentSpawnIndex = INACTIVE_SPAWN_INDEX;
	int totalSpawns = 0;
	bool infinite = false;

	std::vector<int> laneSpawnIndexes = {};

	explicit EnemySpawner(const nlohmann::json& spawnerData);

	// Must only be called AFTER UnitStats is created in teh constructor
	void create_unit_data(const nlohmann::json& spawnerData);

	void forcefully_spawn_an_enemy(Stage& stage);
	void spawn_an_enemy(Stage& stage);
	void unleash_boss_shockwave(Stage& stage) const;

	inline bool can_force_spawn(float time) { return !forcedSpawnTimes.empty() && time > forcedSpawnTimes[0].first; }
};
