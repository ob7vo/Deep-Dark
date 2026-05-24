#include "pch.h"
#include "ArmorySlot.h"
#include "UITextures.h"
#include "Camera.h"
#include "UILayout.h"

using namespace UI::ArmoryMenu;

void ArmorySlot::set_unit(int ID, bool canUse, int c) {
	id = ID; core = c;
	usable = canUse;

	sprite.setTexture(Textures::UI::getUnitSlot(id), true);
}

void ArmorySlot::copy(ArmorySlot& pastedSlot) {
	id = pastedSlot.id; core = pastedSlot.core;
	usable = pastedSlot.usable;

	sprite.setTexture(pastedSlot.sprite.getTexture(), true);
	bounds = sprite.getGlobalBounds().size * 1.05f;
}

void ArmorySlot::set_pos(sf::Vector2f _pos) {
	pos = _pos;
	bounds = sprite.getGlobalBounds().size * 1.05f;
	sprite.setPosition(_pos);
}
bool ArmorySlot::hovered_over(sf::Vector2i mPos) const {
	return static_cast<float>(mPos.x) >= pos.x - bounds.x * 0.5f
		&& static_cast<float>(mPos.x) <= pos.x + bounds.x * 0.5f
		&& static_cast<float>(mPos.y) >= pos.y - bounds.y * 0.5f
		&& static_cast<float>(mPos.y) <= pos.y + bounds.y * 0.5f;
}
std::array<ArmorySlot, UnitConfig::MAX_EQUIP_SLOTS> ArmorySlot::default_armory_loadout(Camera& cam) {
	std::array<ArmorySlot, UnitConfig::MAX_EQUIP_SLOTS> slots;

	for (int i = 0; i < EQUIP_SLOTS; i++) {
		slots[i].set_unit(-1, true);

		slots[i].sprite.setScale(Screen::getSpriteScale(slots[i].sprite, EQUIP_SLOT_SCALE) );
		slots[i].sprite.setOrigin(slots[i].sprite.getLocalBounds().size * 0.5f);
	}

//	slots[0].set_unit(0, 2, true, 0);
	slots[0].set_unit(1, true);
	slots[1].set_unit(0, true);

	return slots;
}