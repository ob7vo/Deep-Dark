#pragma once
#include <json.hpp>
#include <bitset>
#include "StageConfig.h"

struct StageSaveData {
	static nlohmann::json data;

	static bool SaveFileExists();
	static void CreateSaveData();
	static void Load();
	static void Save();

	static int GetStageClears(int stageID);
	static int AddStageClear(int stageID);
	static float GetClearTime(int stageID);
	/// <summary> Sets and Returns the fastest time (old vs new) </summary>
	static float SetClearTime(int stageID, float newTime); 
	static std::bitset<StageConfig::CHALLENGES_PER_STAGE> GetChallengeStatus(int stageID);
	static void ClearChallenge(int stageID, int challengeIndex);
	static void UnlockStages(int stageID);
	static bool StageIsUnlocked(int stageID);
};