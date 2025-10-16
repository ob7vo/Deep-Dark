#include "Observer.h"
#include "StageManager.h"

bool Challenge::notify(StageManager& manager) {
	const UnitRecord& rec = manager.stageRecorder.get_const_unit_record(team);
	switch (challengeType) {
	case ChallengeType::UNIT_DEATHS:
		return compare(rec.deaths[value], value2);
	case ChallengeType::UNITS_SPAWNED:
		return compare(rec.unitsSpawned[value], value2);
	case ChallengeType::DEATHS_VIA_FALLING:
		return compare(rec.deathsByFalling, value);
	case ChallengeType::DEATHS_VIA_SURGES:
		return compare(rec.deathsBySurges, value);
	case ChallengeType::DEATHS_VIA_UNITS:
		return compare(rec.deathsByUnits, value);
	case ChallengeType::DEATHS_VIA_TRAPS:
		return compare(rec.deathsByTraps, value);
	case ChallengeType::DEATHS_VIA_CANNON:
		return compare(rec.deathsByCannons, value);
	case ChallengeType::PARTS_SPENT:
		return compare(manager.stageRecorder.partsSpent, value);
	case ChallengeType::TRAPS_TRIGGERED:
		return compare(manager.stageRecorder.trapsTriggered, value);
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
	case ChallengeType::CHALLENGES_CLEARED:{
			int challengesCleared = 0;
			for (auto& challenge : manager.challenges) {
				if (&challenge == this) continue;
				challengesCleared += challenge.cleared;
			}
			return compare(challengesCleared, value);
		}
	case ChallengeType::CLEAR_TIME:
		return compare(manager.timeSinceStart, value);
	}
	return false;
}