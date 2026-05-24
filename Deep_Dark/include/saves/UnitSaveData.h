#pragma once
#include <json.hpp>

struct UnitSaveData {
	static nlohmann::json data;

	static bool SaveFileExists();
	static void CreateSaveData();
	static void Load();
	static void Save();

	static int GetLevel(int unitID);
	static int IncreaseLevel(int unitID);
	static int IsUnlocked(int unitID);
	static void UnlockUnit(int unitID);
	static bool HasCore(int unitID, int coreID);
	static void UnlockCore(int unitID, int coreID);
};