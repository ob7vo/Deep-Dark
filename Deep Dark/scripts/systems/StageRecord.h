#pragma once
#include "UnitEnums.h"
#include <iostream>
struct StageRecord
{
	// :( = Likely wont be used for a challange
	std::vector<int> arr;
	std::vector<int> laneKills;
	std::vector<int> unitsSpawned;

	int statusEffectProcs = 0; // :(

	int partsEarned = 0; // :(
	int partsSpent = 0;

	int deathsBySurge = 0;
	int deathsByUnits = 0;
	int deathsByFalling = 0;
	int deathByTraps = 0;

	StageRecord(int laneCount) {
		laneKills.reserve(laneCount);
		unitsSpawned.reserve(laneCount);
		laneKills.assign(laneCount, 0);
		unitsSpawned.assign(laneCount, 0);
	}

	inline void add_death(int lane, DeathCause causeOfDeath) {
		laneKills[lane]++;
		switch (causeOfDeath) {
		case DeathCause::FALLING:
			deathsByFalling++;
			break;
		case DeathCause::SURGE:
			deathsBySurge++;
			break;
		case DeathCause::UNIT:
			deathsByUnits++;
			break;
		}
	}
};

