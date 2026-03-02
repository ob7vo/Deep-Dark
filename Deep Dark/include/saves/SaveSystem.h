#pragma once
#include <json_fwd.hpp>
#include <string_view>

const std::string SAVE_FOLDER_PATH = "game_saves/";

struct SaveSystem {
	static const int VERSION = 1;

	static void Initialize(bool overwritePlayerSave = false, bool overwriteStageSave = false, bool overwriteUnitSave = false);
	
	static void LoadAll(); // Loads every file
	static void SaveAll(); // Saves every file

	void load_save(std::string_view file);
};