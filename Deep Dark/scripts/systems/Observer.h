#pragma once
#include "ChallengeTypes.h"
#include <unordered_map>
#include <iostream>

class Unit;
struct Loadout;
struct StageRecord;
struct StageManager;

struct Event {
	//Will have eventType enum for ACHIEVMENTS
	int value = 0;
	void* ptr = nullptr;

	Event() = default;
	Event(int val, void* ptr) : value(val), ptr(ptr) {}
};
struct Observer {
public:
	Observer() = default;
	virtual ~Observer() = default;
	virtual bool notify(const Event& event) = 0;
};
struct Challenge{
	std::string description = "";
	bool cleared = false;
	int value = 0;
	int value2 = 0;

	ChallengeType challengeType = ChallengeType::UNIT_DEATHS;
	ComparisonType comparison = ComparisonType::EQUAL;
	int team = 0;

	Challenge(std::string desc, std::string chaTypeStr, char compTypeChr,
		int team, int val = 0, int val2 = 0) : description(desc),
	value(val), value2(val2), team(team){
		challengeType = get_challenge_type(chaTypeStr);
		comparison = get_comparsion_type(compTypeChr);
	}

	bool notify(StageManager& manager);
		
	inline bool compare(int val, int goal) {
		switch (comparison) {
		case ComparisonType::LESS_THAN: return val < goal;
		case ComparisonType::GREATER_THAN: return val > goal;
		case ComparisonType::EQUAL: return val == goal;
		case ComparisonType::NOT: return val != goal;
		case ComparisonType::BITWISE_AND: return val & goal;
		}
		return false;
	}
	inline CheckType get_check_type(ChallengeType type) {
		int val = static_cast<int>(type);
		if (val <= 5) return CheckType::UNIT_RECORD;
		else if (val <= 7) return CheckType::STAGE_RECORD;
		else if (val <= 9) return CheckType::LOADOUT;
		else return CheckType::STAGE_MANAGER;
	}
	inline ChallengeType get_challenge_type(std::string str) {
		std::transform(str.begin(), str.end(), str.begin(), ::tolower);
		static const std::unordered_map<std::string, ChallengeType> challengeMap = {
			{"unit_deaths", ChallengeType::UNIT_DEATHS},
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
	inline ComparisonType get_comparsion_type(char op) {
		switch (op) {
		case '>': return ComparisonType::GREATER_THAN;
		case '<': return ComparisonType::LESS_THAN;
		case '=': return ComparisonType::EQUAL;
		case '!': return ComparisonType::NOT;
		case '&': return ComparisonType::BITWISE_AND;
		}
	}
};

