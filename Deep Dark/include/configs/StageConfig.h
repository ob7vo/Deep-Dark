#pragma once
#include <json_fwd.hpp>

namespace StageConfig {
	constexpr int TOTAL_STAGES = 3;

	constexpr int CHALLENGES_PER_STAGE = 3;
	constexpr int MAX_PHASES = 3;

	nlohmann::json getStageJson(int stageID);
}