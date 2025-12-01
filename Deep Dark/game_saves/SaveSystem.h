#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <json_fwd.hpp>

const std::string SAVE_FOLDER_PATH = "saves/";
struct SaveSystem{
	static const int VERSION = 1;

	static SaveSystem s_instance;
	inline static SaveSystem& Get() { return s_instance; }

	inline bool saveFileExists(const std::string file) {
		return std::filesystem::exists(SAVE_FOLDER_PATH + file);
	}

	void initialize();
	void create_unit_save();
	void create_stage_save();
	void load_save(const std::string file);
};