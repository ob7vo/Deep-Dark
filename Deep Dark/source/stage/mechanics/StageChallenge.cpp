#include "pch.h"
#include "StageChallenge.h"
#include "StageManager.h"

StageChallenge::StageChallenge(const nlohmann::json& chalJson) :
	description(chalJson.value("description", "")),
	cleared(chalJson.value("clear_state_at_start", false)),
	value(chalJson.value("value", 0)),
	lane(chalJson.value("lane", -1)),
	challengeType(get_challenge_type(chalJson["challenge_type"])),
	comparison(get_comparison_type(chalJson["comparison"].get<std::string>()[0])),
	team(chalJson.value("team", 0))
{
	// pTarget is set by the Manager

	if (chalJson.contains("banned_types"))
		for (auto& target : chalJson["banned_types"])
			value |= static_cast<int>(UnitStats::convert_string_to_type(target));
}

bool StageChallenge::notify(const StageManager& manager) {
	if (pTarget != nullptr) return cleared = compare(*pTarget, value);

	// The other enumerations are simple int value checks
	// handled by the pointer above
	switch (challengeType) {
	case ChallengeType::CLEAR_TIME:
		return cleared = compare((int)manager.stage.timeSinceStart, value);
	case ChallengeType::UNITS_AT_ONCE: {
		return cleared = compare((int)get_current_unit_count(manager), value);
	}
	case ChallengeType::CHALLENGES_CLEARED: {
		int challengesCleared = 0;

		for (const auto& challenge : manager.challenges) {
			if (&challenge == this) continue;
			challengesCleared += (int)challenge.cleared;
		}

		return cleared = compare(challengesCleared, value);
	}
	case ChallengeType::BANNED_UNIT_TYPES: {
		// value will be a bitmask of the banned types
		for (const auto& slot : manager.loadout.slots) 
			if (compare((int)slot.unitStats.unitTypes, value)) return false;

		return false;
	}
	case ChallengeType::UNITS_COST: {
		// If the unit is above or below a cost, return false
		for (const auto& slot : manager.loadout.slots) 
			if (compare(slot.unitStats.parts, value)) return false;
		
		return cleared = true;
	}
	default: return cleared = false;
	}
}

// Getting calues
const int* StageChallenge::get_target_ptr(const StageManager& manager) const {
	UnitRecord& rec = manager.stageRecorder.get_unit_record(team);

	switch (challengeType) {
	case ChallengeType::UNIT_DEATHS:
		if (lane <= 0 || lane >= rec.deaths.size()) return &rec.totalDeaths;
		else return &rec.deaths[lane];
	case ChallengeType::UNITS_SPAWNED:
		if (lane <= 0 || lane >= rec.deaths.size()) return &rec.totalSpawns;
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
	default: return nullptr;
	}
}
size_t StageChallenge::get_current_unit_count(const StageManager& manager) const {
	size_t total = 0;

	if (lane < 0 || lane > manager.stage.laneCount) {
		for (auto& l : manager.stage.lanes)
			total += l.get_unit_count(team);
	}
	else
		total = manager.stage.lanes[lane].get_unit_count(team);

	return total;
}
ComparisonType StageChallenge::get_comparison_type(char op)  {
	switch (op) {
	case '>': return ComparisonType::GREATER_THAN;
	case '<': return ComparisonType::LESS_THAN;
	case '=': return ComparisonType::EQUAL;
	case '!': return ComparisonType::NOT;
	case '&': return ComparisonType::BITWISE_AND;
	case '~': return ComparisonType::BITWISE_NOT;
	default: return ComparisonType::GREATER_THAN;
	}
}
ChallengeType StageChallenge::get_challenge_type(const std::string& str) {
	static const std::unordered_map<std::string, ChallengeType> challengeMap = {
		{"unit_deaths", ChallengeType::UNIT_DEATHS},
		{"units_spawned", ChallengeType::UNITS_SPAWNED},
		{"deaths_by_units", ChallengeType::DEATHS_VIA_UNITS},
		{"deaths_by_surges", ChallengeType::DEATHS_VIA_SURGES},
		{"deaths_by_traps", ChallengeType::DEATHS_VIA_TRAPS},
		{"deaths_by_falling", ChallengeType::DEATHS_VIA_FALLING},
		{"deaths_by_cannons", ChallengeType::DEATHS_VIA_CANNON},
		{"parts_spent", ChallengeType::PARTS_SPENT},
		{"traps_triggered", ChallengeType::TRAPS_TRIGGERED},
		{"banned_unit_types", ChallengeType::BANNED_UNIT_TYPES},
		{"units_cost", ChallengeType::UNITS_COST},
		{"challenges_cleared", ChallengeType::CHALLENGES_CLEARED},
		{"clear_time", ChallengeType::CLEAR_TIME},
	};

	auto it = challengeMap.find(str);
	if (it == challengeMap.end()) std::cout << "Invalid Challenge String" << std::endl;
	return (it != challengeMap.end()) ? it->second : ChallengeType::NONE;
}

bool StageChallenge::compare(int tar, int goal) const {
	switch (comparison) {
	case ComparisonType::LESS_THAN: return tar < goal;
	case ComparisonType::GREATER_THAN: return tar > goal;
	case ComparisonType::EQUAL: return tar == goal;
	case ComparisonType::NOT: return tar != goal;
	case ComparisonType::BITWISE_AND: return tar & goal;
	case ComparisonType::BITWISE_NOT: return !(tar & goal);
	}
	return false;
}