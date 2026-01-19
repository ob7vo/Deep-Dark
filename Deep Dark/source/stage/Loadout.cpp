#include "pch.h"
#include "Loadout.h"
#include "Camera.h"
#include "ArmorySlot.h"
#include "UnitConfig.h"
#include "UITextures.h"

LoadoutSlot::LoadoutSlot(const nlohmann::json& file, const ArmorySlot& unit)
	: empty(false), 
	usauble(unit.usauble),
	slotSprite(Textures::UI::getUnitSlot(unit.id, unit.gear)), 
	unitStats(UnitStats::create_player(file, unit.core)) 
{
	AnimationClip::setup_unit_animation_map(file, aniMap, unitAnimTextures);
	spawnTimer = 0.f; // this is for unitStats-rechargeTime---+
}
Loadout::Loadout(const Camera& cam) {
	set_slot_positions(cam);
}

void Loadout::create_loadout(const std::array<ArmorySlot, 10>& armorySlots) {
	for (int i = 0; i < 10; i++) {
		if (armorySlots[i].id == -1) {
			filledSlots = i;
			break;
		}

		set_slot(armorySlots[i], i);
	}
	for (int j = filledSlots; j < 10; j++)
		slots[j] = LoadoutSlot(Textures::UI::t_defaultUnitSlot);
}
void Loadout::set_slot(const ArmorySlot& unit, int loadoutSlotInd) {
	const nlohmann::json unitJson = UnitConfig::createUnitJson(unit.id, unit.gear);

	slots[loadoutSlotInd] = LoadoutSlot(unitJson, unit);
}
void Loadout::set_slot_positions(const Camera& cam) {
	sf::Vector2f pos = cam.norm_to_pixels(FIRST_SLOT_POS);
	sf::Vector2f inc = cam.norm_to_pixels(SLOT_INCREMENT);
	float startX = pos.x;

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 5; j++) {
			int ind = i * 5 + j;
			LoadoutSlot& slot = slots[ind];
			slot.slotSprite.setPosition(pos);
			slot.slotSprite.setScale({ 2.5f,2.5f });

			pos.x += inc.x;
		}
		pos.x = startX;
		pos.y += inc.y;
	}
}

void Loadout::draw_slots(Camera& cam, int currentParts) {
	for (int i = 0; i < 10; i++)
		slots[i].draw(cam, currentParts);
}
void LoadoutSlot::draw(Camera& cam, int curParts) {
	cam.queue_ui_draw(&slotSprite);

	float fill = 1.f;
	if ((cooldown <= 0 && can_afford_unit(curParts)) || empty) {
		cooldown = 0.f;
		return;
	}
	else if (cooldown > 0)
		fill = (spawnTimer - cooldown) / spawnTimer;

	cam.draw_overlay(slotSprite, UI::Colors::BLACK_TRANSPARENT, fill);
}