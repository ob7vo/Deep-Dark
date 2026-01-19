#pragma once
#include <json_fwd.hpp>
#include <string_view>

const std::string SAVE_FOLDER_PATH = "saves/";

struct SaveSystem{
	static const int VERSION = 1;

	static SaveSystem s_instance;
	inline static SaveSystem& Get() { return s_instance; }

	static bool saveFileExists(std::string_view file);

	static void initialize();

	static void create_player_save();
	static void create_unit_save();
	static void create_stage_save();

	void load_save(std::string_view file);

};