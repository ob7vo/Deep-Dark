#pragma once
#include "json.hpp"
#include "Animation.h"
#include "UnitData.h"
#include <fstream>

static sf::Texture& default_slot_texture() {
	static sf::Texture defaultTex;
	static bool initialized = false;
	if (!initialized) {
		initialized = true;
		(void)defaultTex.loadFromFile("sprites/defaultTexture.png");
	}
	return defaultTex;
}

struct Slot {
	bool filled = false;
	sf::Sprite sprite;

	UnitStats unitStats;
	std::array<Animation, 5> aniMap;
	
	Slot() : filled(false), sprite(default_slot_texture()) {}
	Slot(const nlohmann::json& file, float levelBoost, sf::Texture texture) : unitStats(file, levelBoost), sprite(texture), filled(true){
		Animation::create_unit_animation_array(file, aniMap);
	}
};
struct Loadout{
	std::array<Slot, 10> slots;

	Loadout(std::vector<std::string> jsonPaths) {
		for (size_t i = 0; i < jsonPaths.size(); i++) {
			std::ifstream file(jsonPaths[i]);
			nlohmann::json unitJson = nlohmann::json::parse(file);
			float levelBoost = (float)unitJson["level"];
			levelBoost = (levelBoost + 4) / 5.f;

			/*std::string path = unit["path"];
			std::string texturePath = unit["animations"]["slot_sprite"];
			std::string fullPath = path + texturePath;*/
			sf::Texture baseTexture("sprites/defaultTexture.png");
			slots[i] = Slot(unitJson, levelBoost, baseTexture);
		}
	}
	inline bool filled_slot(int slot) { return slots[slot].filled; }
	inline std::pair<UnitStats*, std::array<Animation, 5>*> get_player_unit_data(int slot) {
		return { &slots[slot].unitStats, &slots[slot].aniMap };
	}
};

