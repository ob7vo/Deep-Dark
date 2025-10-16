#pragma once
#include "UnitEnums.h"
#include "Unit.h"
#include <iostream>

struct StageRecord
{
	// :( = Likely wont be used for a challange
	UnitRecord enemyRecorder;
	UnitRecord playerRecorder;

	int partsEarned = 0;
	int partsSpent = 0;
	int trapsTriggered = 0;

	StageRecord(int lanes) : 
		enemyRecorder(lanes), playerRecorder(lanes) {}

	inline void add_death(int team, int lane, DeathCause causeOfDeath) { 
		get_unit_record(team).add_death(lane, causeOfDeath);
	}
	inline void add_parts_spent(int p) { partsSpent += p; }
	inline void add_parts_earned(int p) { partsEarned += p; }
	inline void add_trap_trigger() { trapsTriggered++; }

	inline const UnitRecord& get_const_unit_record(int team) const {
		return team == 1 ? playerRecorder : enemyRecorder;
	}
	inline UnitRecord& get_unit_record(int team) {
		return team == 1 ? playerRecorder : enemyRecorder;
	}
};
struct UnitRecord {
	std::vector<int> deaths;
	std::vector<int> unitsSpawned;
	
	int deathsBySurges = 0;
	int deathsByUnits = 0;
	int deathsByFalling = 0;
	int deathsByTraps = 0;
	int deathsByCannons = 0;

	UnitRecord(int laneCount) {
		deaths.reserve(laneCount);
		unitsSpawned.reserve(laneCount);
		deaths.assign(laneCount, 0);
		unitsSpawned.assign(laneCount, 0);
	}

	inline void add_death(int lane, DeathCause causeOfDeath) {
		deaths[lane]++;
		switch (causeOfDeath) {
		case DeathCause::UNIT: deathsByUnits++; break;
		case DeathCause::SURGE: deathsBySurges++; break;
		case DeathCause::FALLING: deathsByFalling++; break;
		case DeathCause::TRAP: deathsByTraps++; break;
		case DeathCause::CANNON: deathsByCannons++; break;
		}
	}
};

