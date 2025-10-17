#pragma once
// Challenges normally do NOT have to be completed all in one/the same run.
// CHALLENGES_CLEARED however, would require that for IT'S completetion. 
// (Ex: "Complete 2 (other) challenges in the same run"

enum class CheckType {
	UNIT_RECORD,// <= 5
	STAGE_RECORD, // > 5, <= 7
	LOADOUT, // >= 8, <= 9
	STAGE_MANAGER // >= 10, <= 11
};

enum class ChallengeType {
	NONE = -1,
	UNIT_DEATHS = 0,
	UNITS_SPAWNED = 1,
	DEATHS_VIA_UNITS = 2,
	DEATHS_VIA_SURGES = 3,
	DEATHS_VIA_TRAPS = 4,
	DEATHS_VIA_FALLING = 5,
	DEATHS_VIA_CANNON = 6,
	PARTS_SPENT,
	TRAPS_TRIGGERED,
	BANNED_UNIT_TYPES,
	UNITS_COST,
	CHALLENGES_CLEARED, 
	CLEAR_TIME,
	UNITS_AT_ONCE
};
enum class ComparisonType {
	LESS_THAN = 0, //There will be NO, "or equal to"'s. Just make it exclusive
	GREATER_THAN,
	EQUAL,
	NOT,
	BITWISE_AND,
	BITWISE_NOT
};