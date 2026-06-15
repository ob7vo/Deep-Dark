#pragma once
#include "Animation.h"
#include "UnitLaneTransferRequest.h"
#include "Lane.h"
#include "Base.h"
#include "Spawners.h"
#include "Trap.h"
#include "Teleporter.h"
#include "Projectile.h"
#include "Surge.h"
#include "UnitPool.h"
#include "StageStatus.h"
#include "observers/UnitAbilityObserver.h"
#include "SharedUnitData.h"
#include <iostream>
#include <functional>

const int p_team = 1;
const float FLOOR = 900;

const float HITBOX_TIMER = 0.5f;
const float HITBOX_HEIGHT = 60.f;
const sf::Color HITBOX_COLOR(235, 24, 9, 128);

class Unit;
struct Stage;
struct StageRecord;

struct Stage
{
	UnitPool unitPool;
	StageRecord* recorder = nullptr;
	int nextUnitID = -1;

	std::vector<Lane> lanes = {};
	int laneCount = 0;
	int* selectedLane = 0;

	UnitAbilityObserver unitAbilityObserver;

	std::vector<EnemySpawner> enemySpawners = {};
	std::vector<UnitLaneTransferRequest> laneTransferRequests = {};

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
	// Stored Unit Data (stats and animations)
	// Used for cases where there are extra units that are not stored in an EnemySpawner or the Loadout class
	// Those cases right now are for units that are summoned, and units that transform.
	std::unordered_map<int, std::unique_ptr<SharedUnitData>> unitDataMap;

	Base enemyBase = {};
	Base playerBase = {};
	/// <summary>
	/// Once a team wins, the losing team's units cannot exist.
	/// 0 = ongoing, 1 = PLAYER victory, -1 = ENEMY victory 
	/// </summary>
	StageStatus status;
	std::function<void(bool)> onStageCompletion;

	Stage() = default;
	Stage(const nlohmann::json& stagePhaseJson, StageRecord* recorder);
	Lane& get_closest_lane(float y);

	void break_spawner_thresholds();
	void destroy_base(int baseTeam);

	// Creating Units
	Unit* create_unit(int laneIndex, const UnitStats* unitStats, UnitAnimMap* aniMap);
	void try_revive_unit(UnitSpawner* spawner);
	/// <summary> Reserved for Bosses. They die and tehn transform, like a Phase-Transition </summary>
	void transform_unit(const Unit& unit);
	void summon_construct(const Unit& unit);

	SharedUnitData* get_unit_config(int id, float magnification);

	void try_create_ability_observer(size_t poolIndex, int spawnIndex);

	// Creating Surges
	Surge* create_surge(const Unit& unit, const Augment& surge);
	void create_surge(const BaseCannon* cannon, const Augment& surge);
	void create_surge(const BaseCannon* cannon, const Augment& surge, int lane, float distance);
	Surge* create_surge(const UnitStats* stats, int curLane, int level, sf::Vector2f pos, AugmentType aug);

	void create_projectile(const Unit& unit, const Augment& proj);
	void create_hitbox_visualizers(sf::Vector2f pos, std::pair<float, float> range, int team);

	std::pair<float, int> find_lane_to_fall_on(const Unit& unit, int startLane) const;
	int find_lane_to_knock_to(const Unit& unit, int incrementer) const;

	inline Base& get_enemy_base(int team) { return team == p_team ? enemyBase : playerBase; }

	bool can_queue_lane_transfer_request(int id);
	void queue_lane_transfer_request(Unit& unit, int newLane, float fallTo, UnitLaneTransferRequestType type);

	bool reached_unit_capacity(int team);
	void lower_summons_count(int id);

	inline float clamp_within_lane(float newX, int laneIdx) const {
		auto [minBound, maxBound] = lanes[laneIdx].get_lane_boundaries();
		return std::clamp(newX, minBound, maxBound);
	}

	inline Unit& getUnit(size_t i) { return unitPool.pool[i]; }
	inline const Unit& getUnit(size_t i) const { return unitPool.pool[i]; }
};
