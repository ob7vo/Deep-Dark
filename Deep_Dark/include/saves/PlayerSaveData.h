#pragma once
#include <json.hpp>
#include <optional>
constexpr int MAX_SCRAP_PARTS = 1000000000;

struct PlayerSaveData {
	static nlohmann::json data;

	static bool SaveFileExists();
	static void CreateSaveData();
	static void Load();
	static void Save();

	static int GetScrapParts();
	static std::optional<int> SpendScrapParts(int cost); // Returns if the purchase was successful ro not
	static int GainScrapParts(int parts); // Adds Scrap Parts then returns the new amount.
	static int GetModules();
	static std::optional<int> SpendModules(int cost); // Returns if the purchase was successful ro not
	static int GainModules(int mod); // Adds Modules then returns the new amount.
	static void AddStageClear(); 
	static void AddChallengeClears(int newClears);
};