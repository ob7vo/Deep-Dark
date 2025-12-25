#pragma once
#include "UnitEnums.h"

struct UnitRecord {
	int laneCount = 0;
	std::vector<int> deaths = {};
	int totalDeaths = 0;
	std::vector<int> unitsSpawned = {};
	int totalSpawns = 0;

	int deathsBySurges = 0;
	int deathsByUnits = 0;
	int deathsByFalling = 0;
	int deathsByTraps = 0;
	int deathsByCannons = 0;

	UnitRecord() = default;
	explicit UnitRecord(int lanes) : laneCount(lanes) {
		deaths.reserve(laneCount);
		unitsSpawned.reserve(laneCount);
		deaths.assign(laneCount, 0);
		unitsSpawned.assign(laneCount, 0);
	}
	void set_new_lane_count(int lanes) {
		if (lanes <= this->laneCount) return;
		//If new lanes is less than old LaneCount, do nothing

		laneCount = lanes;
		deaths.resize(lanes, 0);
		unitsSpawned.resize(lanes, 0);
	}

	inline void add_death(int lane, DeathCause causeOfDeath) {
		deaths[lane]++;
		totalDeaths++;

		switch (causeOfDeath) {
		case DeathCause::UNIT: deathsByUnits++; break;
		case DeathCause::SURGE: deathsBySurges++; break;
		case DeathCause::FALLING: deathsByFalling++; break;
		case DeathCause::TRAP: deathsByTraps++; break;
		case DeathCause::CANNON: deathsByCannons++; break;
		case DeathCause::NONE: assert(false && "Unit died yet has no cause of death"); break;
		}
	}
	inline void add_spawn(int lane) {
		unitsSpawned[lane]++;
		totalSpawns++;
	}
};

struct StageRecord
{
	// :( = Likely wont be used for a challange
	mutable UnitRecord enemyRecorder;
	mutable UnitRecord playerRecorder;

	int partsEarned = 0;
	int partsSpent = 0;
	int trapsTriggered = 0;

	StageRecord() = default;
	StageRecord(int lanes) : 
		enemyRecorder(lanes), playerRecorder(lanes) {}
	void set_new_lane_count(int laneCount) {
		playerRecorder.set_new_lane_count(laneCount);
		enemyRecorder.set_new_lane_count(laneCount);
	}
	inline void add_death(int team, int lane, DeathCause causeOfDeath) const { 
		get_unit_record(team).add_death(lane, causeOfDeath);
	}
	inline void add_spawn(int team, int lane) {
		get_unit_record(team).add_spawn(lane);
	}
	inline void add_parts_spent(int p) { partsSpent += p; }
	inline void add_parts_earned(int p) { partsEarned += p; }
	inline void add_trap_trigger() { trapsTriggered++; }

	inline const UnitRecord& get_const_unit_record(int team) const {
		return team == 1 ? playerRecorder : enemyRecorder;
	}
	UnitRecord& get_unit_record(int team) const {
		return team == 1 ? playerRecorder : enemyRecorder;
	}
};

