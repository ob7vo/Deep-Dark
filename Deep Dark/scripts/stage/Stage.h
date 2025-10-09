#pragma once
#include "Animation.h"
#include "Lane.h"
#include "UnitData.h"
#include "Base.h"
#include "Surge.h"
#include "Tween.h"
#include "ActionObject.h"
#include "StageRecord.h"
#include <iostream>

const int p_team = 1;
const float FLOOR = 900;

class Unit;

struct MoveRequest {
	int unitId;
	int currentLane;
	int newLane;
	int team;
	float pos;
	RequestType type;

	MoveRequest(int id, int curLane, int newLane, int team, float fallTo, RequestType type) :
		unitId(id), currentLane(curLane), newLane(newLane), 
		team(team), pos(fallTo), type(type) {}
	MoveRequest(Unit& unit, int newLane, float fallTo, RequestType type);

	inline bool fall_request() const { return type == RequestType::FALL; }
	inline bool teleport_request() const { return type == RequestType::TELEPORT; }
	inline bool squash_request() const { return type == RequestType::SQUASH; }
	inline bool jump_request() const { return type == RequestType::JUMP; }
	inline bool	launch_request() const { return type == RequestType::LAUNCH; }
};
struct EnemySpawner {
	UnitStats enemyStats;
	std::array<Animation, 5> aniArr;

	float nextSpawnTime;
	float firstSpawnTime;
	std::pair<float, float> spawnDelays;
	std::vector<std::pair<float, int>> forcedSpawnTimes;

	int currentSpawnIndex = 0;
	int totalSpawns;
	bool infinite;

	std::vector<int> laneSpawnIndexes;

	EnemySpawner(const nlohmann::json& unitJsonPath, const nlohmann::json& file);

	inline bool can_force_spawn(float time) { return forcedSpawnTimes.size() > 0 && time > forcedSpawnTimes[0].first; }
};
struct Stage
{
	StageRecord& recorder;

	std::vector<Lane> lanes;
	int laneCount = 0;
	int nextUnitID = 0;
	int selectedLane = 0;

	std::vector<EnemySpawner> enemySpawners;
	std::vector<MoveRequest> moveRequests;
	std::vector<std::unique_ptr<Surge>> surges;
	std::vector<std::unique_ptr<ActionObject>> actionObjects;

	std::unordered_map<int, UnitTween> unitTweens;
	std::unique_ptr<Summon> summonData;

	Base enemyBase;
	Base playerBase;

	Stage(const nlohmann::json& stageFile, StageRecord& recorder);

	Unit* create_unit(int laneIndex, const UnitStats* unitStats, std::array<Animation, 5>* aniMap);
	void create_summon(Unit& unit);
	Summon* try_create_summon_data(int summonId, float magnification);
	void try_revive_unit(UnitSpawner* spawner);
	void only_draw(sf::RenderWindow& window);
	void create_surge(Unit& unit, const Augment& surge);
	void create_surge(BaseCannon* pCannon, const Augment& surge);
	void create_surge(BaseCannon* eCannon, const Augment& surge, int lane, float distance);
	void create_surge(const UnitStats* stats, int curLane, int level, sf::Vector2f pos, AugmentType aug);

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
	inline bool tweening(int id) { return unitTweens.contains(id); }
	inline void cancel_tween(int id) { unitTweens.erase(id); }
	inline UnitTween* create_tween(int id, sf::Vector2f startPos, sf::Vector2f endPos, 
		float time, RequestType tweenType, bool overwrite = true) {
		if (tweening(id))
			if (overwrite) cancel_tween(id);
			else return nullptr;

		unitTweens.emplace(id, UnitTween(startPos, endPos, time, tweenType));
		//std::cout << "created tween" << std::endl;
		return &unitTweens[id];
	}
	inline UnitTween* create_tween(Unit& unit, sf::Vector2f endPos, float time,
		RequestType tweenType, bool overwrite = true) {
		return create_tween(unit.id, unit.pos, endPos, time, tweenType, overwrite);
	}
	inline RequestType update_tween(Unit& unit, float deltaTime) {
		unit.pos = unitTweens[unit.id].update_and_get(deltaTime);
		if (unitTweens[unit.id].isComplete) {
			RequestType type = unitTweens[unit.id].tweenType;
			cancel_tween(unit.id);
			return type;
		}
		return RequestType::NOT_DONE;
	}
};
