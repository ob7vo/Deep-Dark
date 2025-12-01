#pragma once
#include "Animation.h"
#include "Lane.h"
#include "UnitData.h"
#include "Base.h"
#include "Spawners.h"
#include "Trap.h"
#include "Teleporter.h"
#include "Surge.h"
#include "Projectile.h"
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

/// <summary>
/// Needed to move Units between vectors (Lanes), and can ONLY be called
/// for that purpose, not just any plain old movement.
/// </summary>
struct MoveRequest {
	int unitId;
	int currentLane;
	int newLane;
	int team;
	float axisPos;
	RequestType type;

	MoveRequest(Unit& unit, int newLane, float axisPos, RequestType type);

	void move_unit_by_request(Unit& unit, Stage& stage) const;
	inline bool fall_request() const { return type == RequestType::FALL; }
	inline bool teleport_request() const { return type == RequestType::TELEPORT; }
	inline bool squash_request() const { return type == RequestType::SQUASH; }
	inline bool jump_request() const { return type == RequestType::JUMP; }
	inline bool	launch_request() const { return type == RequestType::LAUNCH; }
	
};

struct Stage
{
	StageRecord* recorder = nullptr;

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
	Stage(const nlohmann::json& stageFile, int stageSet, StageRecord* recorder);
	Lane& get_closest_lane(float y);

	Unit* create_unit(int laneIndex, const UnitStats* unitStats, UnitAniMap* aniMap);
	void create_summon(Unit& unit);
	bool can_summon(int summonId, float magnification);
	void try_revive_unit(UnitSpawner* spawner);
	void break_spawner_thresholds(float timeSinceStart = 0.f);

	Surge* create_surge(Unit& unit, const Augment& surge);
	void create_surge(BaseCannon* pCannon, const Augment& surge);
	void create_surge(BaseCannon* eCannon, const Augment& surge, int lane, float distance);
	Surge* create_surge(const UnitStats* stats, int curLane, int level, sf::Vector2f pos, AugmentType aug);
	void create_projectile(Unit& unit, const Augment& proj);
	void create_hitbox_visualizers(sf::Vector2f pos, std::pair<float, float> range, int team);

	std::pair<float, int> find_lane_to_fall_on(Unit& unit);
	int find_lane_to_knock_to(Unit& unit, int incrementer) const;

	inline std::vector<Unit>& get_lane_targets(int i, int team) { return lanes[i].get_targets(team); }
	inline std::vector<Unit>& get_source_vector(int i, int team) { return lanes[i].get_source(team); }
	inline Lane& get_lane(int i) { return lanes[i]; }
	inline Base& get_enemy_base(int team) { return team == p_team ? enemyBase : playerBase; }

	inline bool over_gap(int i, float xPos) { return lanes[i].within_gap(xPos); }
	inline bool out_of_lane(int i, float xPos) { return lanes[i].out_of_lane(xPos); }
	inline float get_team_wall(int i, int team) { return lanes[i].get_wall(team); }
	inline std::pair<float, float> get_walls(int i) { return { lanes[i].get_wall(1), lanes[i].get_wall(-1) }; }

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
