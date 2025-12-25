#include "pch.h"
#include "ArmorySlot.h"
#include "UITextures.h"

void ArmorySlot::set_unit(int ID, int g, bool canUse, int c) {
	id = ID; gear = g; core = c;
	usauble = canUse;

	sprite.setTexture(Textures::UI::getUnitSlot(id, gear), true);
}
void ArmorySlot::set_unit(std::pair<int, int> unit, bool canUse) {
	set_unit(unit.first, unit.second, canUse);
}
void ArmorySlot::set_pos(sf::Vector2f _pos) {
	pos = _pos;
	bounds = bounds = sprite.getGlobalBounds().size * 1.05f;
	sprite.setPosition(_pos);
}
bool ArmorySlot::hovered_over(sf::Vector2i mPos) const {
	return static_cast<float>(mPos.x) >= pos.x - bounds.x * 0.5f
		&& static_cast<float>(mPos.x) <= pos.x + bounds.x * 0.5f
		&& static_cast<float>(mPos.y) >= pos.y - bounds.y * 0.5f
		&& static_cast<float>(mPos.y) <= pos.y + bounds.y * 0.5f;
}
std::array<ArmorySlot, 10> ArmorySlot::default_armory_loadout() {
	std::array<ArmorySlot, 10> slots;

	slots[0].set_unit(0, 3, true, 0);
	slots[1].set_unit(1, 2, true);
	for (int i = 2; i < 10; i++) slots[i].set_unit(-1, 1, true);

	return slots;
}