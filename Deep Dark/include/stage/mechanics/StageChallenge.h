#pragma once
#include "ChallengeTypes.h"
#include <json_fwd.hpp>

class Unit;
struct Loadout;
struct StageRecord;
struct StageManager;

struct StageChallenge {
	std::string description = "";
	bool cleared = false;
	int value = 0;
	// A value other than 0 (and within the laneCount) means the challenge only apples to a specific lane
	int laneInd = 0; 
	const int* pTarget = nullptr;

	ChallengeType challengeType = ChallengeType::UNIT_DEATHS;
	ComparisonType comparison = ComparisonType::EQUAL;
	int team = 0;

	explicit StageChallenge(const nlohmann::json& challengesJson);

	bool notify(const StageManager& manager);

	bool compare(int tar, int goal) const;
	static ComparisonType get_comparison_type(char op);
	static ChallengeType get_challenge_type(const std::string& str);

	size_t get_current_unit_count(const StageManager& manager) const;
	const int* get_target_ptr(const StageManager& manager) const;
};

