#include "pch.h"
#include "StageConfig.h"

namespace StageConfig {
	nlohmann::json getStageJson(int stageID) {
		std::string jsonPath = std::format("configs/stage_data/stage_{}.json", stageID);
		std::ifstream stageFile(jsonPath);

		nlohmann::json stageJson = nlohmann::json::parse(stageFile);
		stageFile.close();

		return stageJson;
	}
}