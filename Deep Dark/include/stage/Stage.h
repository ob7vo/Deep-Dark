#pragma once
#include "Animation.h"
#include "UnitMoveRequest.h"
#include "Lane.h"
#include "UnitData.h"
#include "Base.h"
#include "Spawners.h"
#include "Trap.h"
#include "Teleporter.h"
#include "Projectile.h"
#include "Surge.h"
#include <iostream>

const int p_team = 1;
const float FLOOR = 900;
const float INACTIVE_SPAWNER = 10000.f;

const float HITBOX_TIMER = 0.5f;
const float HITBOX_HEIGHT = 60.f;
const sf::Color HITBOX_COLOR(235, 24, 9, 128);

class Unit;
struct Stage;
struct StageRecord;

struct SummonData {
	int count = 0;
	const UnitStats stats;
	UnitAniMap ani;

	SummonData(const nlohmann::json& file, float mag) :
		stats(UnitStats::enemy(file, mag)) {
	}
}; 

struct Stage
{
	StageRecord* recorder = nullptr;
	float timeSinceStart = 0.f;

	std::vector<Lane> lanes = {};
	int laneCount = 0;
	int nextUnitID = 0;
	int selectedLane = 0;

	std::vector<EnemySpawner> enemySpawners = {};
	std::vector<MoveRequest> moveRequests = {};

	std::vector<std::pair<AugmentType, sf::Vector2f>> effectSpritePositions;
	std::vector<std::pair<sf::RectangleShape, float>> hitboxes;

	std::vector<std::unique_ptr<Surge>> surges = {}; // Temporary
	std::vector<std::unique_ptr<StageEntity>> entities = {}; // Temporary
	std::vector<Projectile> projectiles = {}; // Temporary
	std::vector<Teleporter> teleporters = {}; // Persistant
	std::vector<Trap> traps = {}; // Persistant

	std::unordered_map<int, ProjectileConfig> projConfigs = {};
	std::unique_ptr<SummonData> summonData = nullptr;

	Base enemyBase = {};
	Base playerBase = {};

	Stage() = default;
	Stage(const nlohmann::json& stageSetJson, StageRecord* recorder);
	Lane& get_closest_lane(float y);

	void break_spawner_thresholds();

	// Creating Units
	Unit* create_unit(int laneIndex, const UnitStats* unitStats, UnitAniMap* aniMap);
	void try_revive_unit(UnitSpawner* spawner);
	void create_summon(const Unit& unit);
	bool can_summon(int summonId, float magnification);

	// Creating Surges
	Surge* create_surge(const Unit& unit, const Augment& surge);
	void create_surge(const BaseCannon* cannon, const Augment& surge);
	void create_surge(const BaseCannon* cannon, const Augment& surge, int lane, float distance);
	Surge* create_surge(const UnitStats* stats, int curLane, int level, sf::Vector2f pos, AugmentType aug);

	void create_projectile(const Unit& unit, const Augment& proj);
	void create_hitbox_visualizers(sf::Vector2f pos, std::pair<float, float> range, int team);

	std::pair<float, int> find_lane_to_fall_on(const Unit& unit);
	int find_lane_to_knock_to(const Unit& unit, int incrementer) const;

	inline std::vector<Unit>& get_lane_targets(int i, int team) { return lanes[i].getOpponentUnits(team); }
	inline std::vector<Unit>& get_source_vector(int i, int team) { return lanes[i].getAllyUnits(team); }
	inline Base& get_enemy_base(int team) { return team == p_team ? enemyBase : playerBase; }

	inline bool can_push_move_request(int id) {
		auto it = std::find_if(moveRequests.begin(), moveRequests.end(),
			[&](const MoveRequest& req) { return req.unitId == id; });

		return it == moveRequests.end();
	}
	inline void push_move_request(Unit& unit, int newLane, float fallTo, RequestType type) {
		if (!can_push_move_request(unit.id)) return;
		moveRequests.emplace_back(unit, newLane, fallTo, type);
	}

};
