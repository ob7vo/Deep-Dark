#pragma once
#include <json.hpp>
#include "Animation.h"
#include "UnitData.h"
#include "Camera.h"
#include "ArmorySlot.h"
#include <fstream>

const sf::Vector2f FIRST_SLOT_POS = { 0.278f, 0.84375f };
const sf::Vector2f SLOT_INCREMENT = { 0.089f, 0.0625f };

struct LoadoutSlot {
	bool empty = true;
	bool autoDeploy = false;
	float cooldown = 0.f;
	float spawnTimer = 0.f;

	sf::Sprite slotSprite = sf::Sprite(defTexture);
	float left = 0;
	float top = 0;
	float width = 0;
	float height = 0;

	UnitStats unitStats = {};
	UnitAniMap aniMap = {};
	
	LoadoutSlot() = default;
	explicit LoadoutSlot(sf::Texture& tex) : slotSprite(tex) { set_bounds(); }
	LoadoutSlot(const nlohmann::json& file, sf::Texture& tex, int core)
		: slotSprite(tex), empty(false),
		unitStats(UnitStats::player(file, core)) {
		Animation::setup_unit_animation_map(file, aniMap);
		set_bounds();
		spawnTimer = unitStats.rechargeTime;
	}
	inline void set_bounds() {
		sf::FloatRect bounds = slotSprite.getGlobalBounds();
		left = bounds.position.x;
		top = bounds.position.y;
		height = bounds.size.y;
		width = bounds.size.x;
	}

	inline void draw_cooldown_bar(Camera& cam, float percentage) const {
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
	inline bool can_afford_unit(int parts) const { return unitStats.parts <= parts; }
};
struct Loadout{
	std::array<LoadoutSlot, 10> slots;
	int filledSlots = 0;

	sf::Texture defaultSlotTexture;
	std::vector<sf::Texture> slotTextures = {};
	
	explicit Loadout(Camera& cam) { 
		slotTextures.reserve(10);
		set_slot_positions(cam); 
		if (!defaultSlotTexture.loadFromFile("sprites/defaults/empty_slot.png"))
			std::cout << "empty_slot.png was not found\n";
	}
	inline void set_slot_positions(Camera& cam) {
		sf::Vector2f pos = cam.norm_to_pixels(FIRST_SLOT_POS);
		sf::Vector2f inc = cam.norm_to_pixels(SLOT_INCREMENT);
		float startX = pos.x;

		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 5; j++) {
				int ind = i * 5 + j;
				LoadoutSlot& slot = slots[ind];
				slot.slotSprite.setPosition(pos);
				slot.slotSprite.setScale({ 2.5f,2.5f });
				slot.set_bounds();
				pos.x += inc.x;
			}
			pos.x = startX;
			pos.y += inc.y;
		}
	}

	void create_loadout(std::array<ArmorySlot, 10> armorySlots) {
		slotTextures = {};


		for (int i = 0; i < 10; i++) {
			ArmorySlot& slot = armorySlots[i];
			if (slot.id == -1) {
				filledSlots = i;
				break;
			}

			set_slot(slot.id, slot.gear, slot.core, i);
		}
		for (int j = filledSlots; j < 10; j++)
			slots[j] = LoadoutSlot(defaultSlotTexture);
	}

	inline void set_slot(int id, int gear, int coreInd, int slot) {
		const nlohmann::json unitJson = UnitData::get_unit_json(id, gear);

		slotTextures.emplace_back(UnitData::get_slot_texture(id, gear));
		slots[slot] = LoadoutSlot(unitJson, slotTextures.back(), coreInd);
	}
	inline void draw_slots(Camera& cam, int currentParts) {
		for (int i = 0; i < 10; i++) 
			slots[i].draw(cam, currentParts);
	}
	inline bool empty_slot(int slot) { return slots[slot].empty; }
};

