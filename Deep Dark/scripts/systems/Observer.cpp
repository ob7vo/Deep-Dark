#include "Observer.h"
#include "StageManager.h"

bool Challenge::notify(StageManager& manager) {
	if (pTarget != nullptr) return compare(*pTarget, value);

	switch (challengeType) {
	case ChallengeType::CLEAR_TIME:
		return compare((int)manager.timeSinceStart, value);
	case ChallengeType::UNITS_AT_ONCE: {
		size_t totalUnitsOfTeam = get_current_unit_count(manager);
		return compare((int)totalUnitsOfTeam, value);
	}
	case ChallengeType::CHALLENGES_CLEARED: {
		int challengesCleared = 0;
		for (auto& challenge : manager.challenges) {
			if (&challenge == this) continue;
			challengesCleared += challenge.cleared;
		}
		return compare(challengesCleared, value);
	}
	case ChallengeType::BANNED_UNIT_TYPES: {
		// value will be a bitmask of types
		for (auto& slot : manager.loadout.slots) {
			int types = slot.unitStats.unitTypes;
			int bannedTypes = value;
			if (compare(types, bannedTypes)) return false;
		}
		return false;
	}
	case ChallengeType::UNITS_COST: {
		for (auto& slot : manager.loadout.slots) {
			int cost = slot.unitStats.parts;
			if (compare(cost, value)) return false;
		}
		return false;
	}
	}
	return false;
}
int* Challenge::get_target_ptr(StageManager& manager) {
	UnitRecord& rec = manager.stageRecorder.get_unit_record(team);

	switch (challengeType) {
	case ChallengeType::UNIT_DEATHS:
		if (lane <= 0 || lane >= rec.deaths.size()) return &rec.totalDeaths;
		else return &rec.deaths[lane];
	case ChallengeType::UNITS_SPAWNED:
		if (lane <= 0 || lane >= rec.deaths.size()) {
			std::cout << "getting total spawns address: " << &rec.totalSpawns << std::endl;
			return &rec.totalSpawns;
		}
		else return &rec.unitsSpawned[lane];
	case ChallengeType::DEATHS_VIA_FALLING:
		return &rec.deathsByFalling;
	case ChallengeType::DEATHS_VIA_SURGES:
		return &rec.deathsBySurges;
	case ChallengeType::DEATHS_VIA_UNITS:
		return &rec.deathsByUnits;
	case ChallengeType::DEATHS_VIA_TRAPS:
		return &rec.deathsByTraps;
	case ChallengeType::DEATHS_VIA_CANNON:
		return &rec.deathsByCannons;
	case ChallengeType::PARTS_SPENT:
		return &manager.stageRecorder.partsSpent;
	case ChallengeType::TRAPS_TRIGGERED:
		return &manager.stageRecorder.trapsTriggered;
	}

	return nullptr;
}
size_t Challenge::get_current_unit_count(StageManager& manager) {
	size_t total = 0;
	if (lane < 0 || lane > manager.stage.laneCount) {
		for (auto& l : manager.stage.lanes)
			total += l.get_unit_count(team);
	}
	else
		total = manager.stage.lanes[lane].get_unit_count(team);

	return total;
}