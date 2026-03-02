#include "pch.h"
#include "Loadout.h"
#include "Camera.h"
#include "ArmorySlot.h"
#include "UnitConfig.h"
#include "UITextures.h"

LoadoutSlot::LoadoutSlot(const nlohmann::json& file, const ArmorySlot& equipSlot)
	: empty(false), 
	usable(equipSlot.usable),
	slotSprite(Textures::UI::getUnitSlot(equipSlot.id, equipSlot.gear)),
	unitStats(UnitStats::create_player(file, equipSlot.core))
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
void Loadout::set_slot(const ArmorySlot& equipSlot, int loadoutSlotInd) {
	const nlohmann::json unitJson = UnitConfig::createUnitJson(equipSlot.id, equipSlot.gear);

	slots[loadoutSlotInd] = LoadoutSlot(unitJson, equipSlot);
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
	cam.renderer.queue_ui_draw(&slotSprite);

	float fill = 1.f;
	if ((cooldown <= 0 && can_afford_unit(curParts)) || empty) {
		cooldown = 0.f;
		return;
	}
	else if (cooldown > 0)
		fill = (spawnTimer - cooldown) / spawnTimer;

	cam.renderer.draw_overlay(slotSprite, UI::Colors::BLACK_TRANSPARENT, fill);
}