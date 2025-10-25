#pragma once
#include "json.hpp"
#include "Animation.h"
#include "UnitData.h"
#include "Camera.h"
#include <fstream>

static sf::Texture& default_slot_texture() {
	static sf::Texture defaultTex;
	static bool initialized = false;
	if (!initialized) {
		initialized = true;
		(void)defaultTex.loadFromFile("sprites/defaults/empty_slot.png");
	}
	return defaultTex;
}

struct Slot {
	bool empty = true;
	float cooldown = 0.f;
	float spawnTimer = 0.f;
	sf::Sprite slotSprite;
	float left = 0;
	float top = 0;
	float width = 0;
	float height = 0;

	UnitStats unitStats;
	std::array<Animation, 5> aniMap;
	
	Slot() : empty(true), slotSprite(default_slot_texture()) { set_bounds(); }
	Slot(const nlohmann::json& file, float levelBoost, sf::Texture& tex, float timer)
		: unitStats(file, levelBoost), spawnTimer(timer), cooldown(0.f),
		slotSprite(tex), empty(false) {
		Animation::create_unit_animation_array(file, aniMap);
		set_bounds();
	}
	inline void set_bounds() {
		sf::FloatRect bounds = slotSprite.getGlobalBounds();
		left = bounds.position.x;
		top = bounds.position.y;
		height = bounds.size.y;
		width = bounds.size.x;
	}

	inline void draw_cooldown_bar(Camera& cam, float percentage) {
		//std::cout << "drawing bar with percentage: " << percentage << std::endl;
		sf::VertexArray darkOverlay(sf::PrimitiveType::TriangleStrip, 4);
		float darkHeight = height * (1.0f - percentage);  // Fill from bottom

		darkOverlay[0].position = { left, top + height - darkHeight };
		darkOverlay[0].color = blackTransperent; // Semi-transparent black

		darkOverlay[1].position = { left + width, top + height - darkHeight };
		darkOverlay[1].color = blackTransperent;

		darkOverlay[2].position = { left, top + height };
		darkOverlay[2].color = blackTransperent;

		darkOverlay[3].position = { left + width, top + height };
		darkOverlay[3].color = blackTransperent;

		cam.queue_temp_ui_draw(darkOverlay);
	}
	inline void draw(Camera& cam, int curParts) {
		cam.queue_ui_draw(&slotSprite);

		float percentage = 0;
		if ((cooldown <= 0 && can_afford_unit(curParts)) || empty) {
			cooldown = 0.f;
			return;
		}
		else if (cooldown > 0) 
			percentage = (spawnTimer - cooldown) / spawnTimer;

		draw_cooldown_bar(cam, percentage);
	}
	inline bool can_afford_unit(int parts) { return unitStats.parts <= parts; }
};
struct Loadout{
	std::array<Slot, 10> slots;
	std::vector<sf::Texture> slotTextures = {};
	int filledSlots = 0;
	
	Loadout(Camera& cam) { set_slot_positions(cam); }
	inline void set_slot_positions(Camera& cam) {
		sf::Vector2f pos = cam.norm_to_pixels(UI::Stage::FIRST_UNIT_SLOT_POS);
		sf::Vector2f inc = cam.norm_to_pixels(UI::Stage::UNIT_SLOT_INCREMENT);
		float startX = pos.x;
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 5; j++) {
				int ind = i * 5 + j;
				Slot& slot = slots[ind];
				slot.slotSprite.setPosition(pos);
				slot.slotSprite.setScale({ 2.5f,2.5f });
				slot.set_bounds();
				pos.x += inc.x;
			}
			pos.x = startX;
			pos.y += inc.y;
		}
	}
	void create_loadout(std::vector<std::string> jsonPaths) {
		slotTextures = {};
		filledSlots = (int)jsonPaths.size();

		for (size_t i = 0; i < filledSlots; i++)
			set_slot(jsonPaths[i], i);
		for (size_t j = filledSlots; j < 10; j++)
			slots[j] = {};
	}
	inline void set_slot(std::string jsonPath, int slot) {
		std::ifstream file(jsonPath);
		nlohmann::json unitJson = nlohmann::json::parse(file);
		float levelBoost = (float)unitJson["level"];
		float timer = unitJson["recharge_timer"];
		levelBoost = (levelBoost + 4) / 5.f;

		std::string path = unitJson["path"];
		std::string fileName = unitJson["slot_sprite"];
		std::string fullPath = path + fileName;
		slotTextures.emplace_back(sf::Texture(fullPath));
		slots[slot] = Slot(unitJson, levelBoost, slotTextures.back(), timer);
	}
	inline void draw_slots(Camera& cam, int currentParts) {
		for (int i = 0; i < 10; i++) {
			slots[i].draw(cam, currentParts);
		}
	}
	inline bool empty_slot(int slot) { return slots[slot].empty; }
	inline std::pair<UnitStats*, std::array<Animation, 5>*> get_player_unit_data(int slot) {
		return { &slots[slot].unitStats, &slots[slot].aniMap };
	}
};

