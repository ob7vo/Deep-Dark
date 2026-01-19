#pragma once
#include "Animation.h"
#include "UnitStats.h"
#include <deque>

const sf::Vector2f FIRST_SLOT_POS = { 0.278f, 0.84375f };
const sf::Vector2f SLOT_INCREMENT = { 0.089f, 0.0625f };

class Camera;
struct ArmorySlot;

struct LoadoutSlot {
	bool empty = true;
	bool usauble = true;
	bool autoDeploy = false;

	float cooldown = 0.f;
	float spawnTimer = 0.f;

	sf::Sprite slotSprite = sf::Sprite(defTex);

	UnitStats unitStats = {};

	std::deque<sf::Texture> unitAnimTextures = {};
	UnitAniMap aniMap = {};
	
	LoadoutSlot() = default;
	explicit LoadoutSlot(const sf::Texture& tex) : slotSprite(tex) { }
	LoadoutSlot(const nlohmann::json& file, const ArmorySlot& unitSlot);

	void draw(Camera& cam, int curParts);
	inline bool can_afford_unit(int parts) const { return unitStats.parts <= parts; }
};
struct Loadout{
	std::array<LoadoutSlot, 10> slots;
	int filledSlots = 0;

	explicit Loadout(const Camera& cam);

	void set_slot_positions(const Camera& cam);
	void create_loadout(const std::array<ArmorySlot, 10>& armorySlots);
	void set_slot(const ArmorySlot& armorySlot, int loadoutSlot);

	void draw_slots(Camera& cam, int currentParts);

	inline bool empty_slot(int slot) { return slots[slot].empty; }
};

