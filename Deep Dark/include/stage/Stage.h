#pragma once
#include "Animation.h"
#include "UnitMoveRequest.h"
#include "Lane.h"
#include "Base.h"
#include "Spawners.h"
#include "Trap.h"
#include "Teleporter.h"
#include "Projectile.h"
#include "Surge.h"
#include "UnitPool.h"
#include <iostream>

const int p_team = 1;
const float FLOOR = 900;

const float HITBOX_TIMER = 0.5f;
const float HITBOX_HEIGHT = 60.f;
const sf::Color HITBOX_COLOR(235, 24, 9, 128);


class Unit;
struct Stage;
struct StageRecord;

struct UnitData {
	/// <summary> How many Unit Instances are using this data </summary>
	int count = 0;
	const UnitStats stats;
	UnitAniMap ani;

	UnitData(const nlohmann::json& file, float mag) :
		stats(UnitStats::create_enemy(file, mag)) {
	}
}; 

struct Stage
{
	UnitPool unitPool;

	StageRecord* recorder = nullptr;
	float timeSinceStart = 0.f;
	int laneCount = 0;
	int nextUnitID = -1;
	// Lane Management
	std::vector<Lane> lanes = {};
	int* selectedLane = 0;

	std::vector<EnemySpawner> enemySpawners = {};
	std::vector<UnitMoveRequest> moveRequests = {};

	// Effects
	std::vector<std::pair<AugmentType, sf::Vector2f>> effectSpritePositions;
	std::vector<std::pair<sf::RectangleShape, float>> hitboxes;

	// Entities
	std::vector<std::unique_ptr<Surge>> surges = {}; // Temporary
	std::vector<std::unique_ptr<StageEntity>> entities = {}; // Temporary
	std::vector<Projectile> projectiles = {}; // Temporary
	std::vector<Teleporter> teleporters = {}; // Persistant
	std::vector<Trap> traps = {}; // Persistant

	// Configs
	std::unordered_map<int, ProjectileData> projDataMap = {};
	std::unordered_map<int, std::unique_ptr<UnitData>> unitDataMap;

	Base enemyBase = {};
	Base playerBase = {};
	/// <summary>
	/// Once a team wins, the losing team's units cannot exist.
	/// 0 = ongoing, 1 = PLAYER victory, -1 = ENEMY victory 
	/// </summary>
	int victoriousTeam = 0;

	Stage() = default;
	Stage(const nlohmann::json& stageSetJson, StageRecord* recorder);
	Lane& get_closest_lane(float y);

	void break_spawner_thresholds();
	void destroy_base(int baseTeam);

	// Creating Units
	Unit* create_unit(int laneIndex, const UnitStats* unitStats, UnitAniMap* aniMap);
	void try_revive_unit(UnitSpawner* spawner);
	/// <summary> Reserved for Bosses. They die and tehn transform, like a Phase-Transition </summary>
	void transform_unit(const Unit& unit);
	void create_summon(const Unit& unit);
	UnitData* get_unit_config(int id, float magnification, UnitSpawnType spawnType);

	// Creating Surges
	Surge* create_surge(const Unit& unit, const Augment& surge);
	void create_surge(const BaseCannon* cannon, const Augment& surge);
	void create_surge(const BaseCannon* cannon, const Augment& surge, int lane, float distance);
	Surge* create_surge(const UnitStats* stats, int curLane, int level, sf::Vector2f pos, AugmentType aug);

	void create_projectile(const Unit& unit, const Augment& proj);
	void create_hitbox_visualizers(sf::Vector2f pos, std::pair<float, float> range, int team);

	std::pair<float, int> find_lane_to_fall_on(const Unit& unit) const;
	int find_lane_to_knock_to(const Unit& unit, int incrementer) const;

	inline Base& get_enemy_base(int team) { return team == p_team ? enemyBase : playerBase; }

	bool can_push_move_request(int id);
	void push_move_request(Unit& unit, int newLane, float fallTo, UnitMoveRequestType type);

	bool reached_unit_capacity(int team);
	void lower_summons_count(int id);

	inline float clamp_within_lane(float newX, int laneInd) const {
		auto [minBound, maxBound] = lanes[laneInd].get_lane_boundaries();
		return std::clamp(newX, minBound, maxBound);
	}

	inline Unit& getUnit(size_t i) { return unitPool.pool[i]; }
	inline const Unit& getUnit(size_t i) const { return unitPool.pool[i]; }
};
