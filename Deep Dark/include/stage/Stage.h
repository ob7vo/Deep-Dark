#pragma once
#include "Animation.h"
#include "Lane.h"
#include "UnitData.h"
#include "Base.h"
#include "Surge.h"
#include "Tween.h"
#include "ActionObject.h"
#include "StageRecord.h"
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

struct SummonData {
	int count = 0;
	const UnitStats stats;
	UnitAniMap ani;

	SummonData(const nlohmann::json& file, float mag) :
		stats(UnitStats::enemy(file, mag)) {
	}
}; 
struct MoveRequest {
	int unitId;
	int currentLane;
	int newLane;
	int team;
	float axisPos;
	RequestType type;

	MoveRequest(int id, int curLane, int newLane, 
		int team, float axisPos, RequestType type) :
		unitId(id), currentLane(curLane), newLane(newLane), 
		team(team), axisPos(axisPos), type(type) {}
	MoveRequest(Unit& unit, int newLane, float axisPos, RequestType type);

	void move_unit_by_request(Unit& unit, Stage& stage);
	inline bool fall_request() const { return type == RequestType::FALL; }
	inline bool teleport_request() const { return type == RequestType::TELEPORT; }
	inline bool squash_request() const { return type == RequestType::SQUASH; }
	inline bool jump_request() const { return type == RequestType::JUMP; }
	inline bool	launch_request() const { return type == RequestType::LAUNCH; }
	
};
struct EnemySpawner {
	UnitStats enemyStats;
	UnitAniMap aniArr;

	float nextSpawnTime = 10000.f;
	float percentThreshold = 101.f;
	std::pair<float, float> spawnDelays;
	///<summary>First is the EXACT time an enemy will spawn, Second is the lane </summary>
	std::vector<std::pair<float, int>> forcedSpawnTimes;

	int currentSpawnIndex = -1;
	int totalSpawns = 0;
	bool infinite = false;

	std::vector<int> laneSpawnIndexes;

	EnemySpawner(const nlohmann::json& spawnerData, Stage& stage);

	inline bool can_force_spawn(float time) { return forcedSpawnTimes.size() > 0 && time > forcedSpawnTimes[0].first; }
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
	std::vector<Projectile> projectiles = {};

	std::vector<std::pair<AugmentType, sf::Vector2f>> effectSpritePositions;
	std::vector<std::pair<sf::RectangleShape, float>> hitboxes;
	std::vector<std::unique_ptr<Surge>> surges = {};
	std::vector<std::unique_ptr<ActionObject>> actionObjects = {};

	std::unordered_map<int, ProjectileConfig> projConfigs = {};
	std::unique_ptr<SummonData> summonData = nullptr;

	Base enemyBase = {};
	Base playerBase = {};

	Stage() = default;
	Stage(const nlohmann::json& stageFile, StageRecord* recorder);
	Lane& get_closest_lane(float y);

	Unit* create_unit(int laneIndex, const UnitStats* unitStats, UnitAniMap* aniMap);
	void create_summon(Unit& unit);
	SummonData* try_get_summon_data(int summonId, float magnification);
	void try_revive_unit(UnitSpawner* spawner);
	void break_spawner_thresholds(float timeSinceStart = 0.f);

	Surge* create_surge(Unit& unit, const Augment& surge);
	void create_surge(BaseCannon* pCannon, const Augment& surge);
	void create_surge(BaseCannon* eCannon, const Augment& surge, int lane, float distance);
	Surge* create_surge(const UnitStats* stats, int curLane, int level, sf::Vector2f pos, AugmentType aug);
	void create_projectile(Unit& unit, const Augment& proj);
	void create_hitbox_visualizers(sf::Vector2f pos, std::pair<float, float> range, int team);

	std::pair<float, int> find_lane_to_fall_on(Unit& unit);
	int find_lane_to_knock_to(Unit& unit, int incrementer);

	inline std::vector<Unit>& get_lane_targets(int i, int team) { return lanes[i].get_targets(team); }
	inline std::vector<Unit>& get_source_vector(int i, int team) { return lanes[i].get_source(team); }
	inline Lane& get_lane(int i) { return lanes[i]; }
	inline Base& get_enemy_base(int team) { return team == p_team ? enemyBase : playerBase; }
	inline bool can_teleport(sf::Vector2f pos, int laneIndex, int team) {
		Teleporter* tp = lanes[laneIndex].get_teleporter(team);
		return tp && tp->check_if_on_teleporter(pos.x, pos.y);
	}
	inline bool over_gap(int i, float xPos) { return lanes[i].within_gap(xPos); }
	inline bool out_of_lane(int i, float xPos) { return lanes[i].out_of_lane(xPos); }
	inline float get_team_wall(int i, int team) { return lanes[i].get_wall(team); }
	inline std::pair<float, float> get_walls(int i) { return { lanes[i].get_wall(1), lanes[i].get_wall(-1) }; }
	inline void push_move_request(int id, int currentLane, int newLane, int team, float cord, RequestType type) {
		moveRequests.emplace_back(id, currentLane, newLane, team, cord, type);
	}
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
