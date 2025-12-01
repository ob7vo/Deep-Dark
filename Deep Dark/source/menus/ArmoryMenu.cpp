#include "pch.h"
#include "ArmoryMenu.h"
#include "UnitData.h"
#include "UILayout.h"
#include "ArmorySlot.h"
#include "Camera.h"
#include "WorkshopMenu.h"

using namespace UI::ArmoryMenu;
using namespace UI::Colors;
using namespace UnitData;

ArmoryMenu::ArmoryMenu(Camera& cam) : Menu(cam), stageSetMenu(cam) {
	slots = ArmorySlot::default_armory_loadout();

	std::vector<Button*> btns = {};
	for (int id = 0; id < 3; id++) {
		Button& btn = buttonManager.buttons[id];
		btns.push_back(&btn);

		btn.setup(UI::ZERO, SLIDER_SLOT_SCLAE, cam, TextureManager::getUnitSlot(id, 1));
		unitSelectionGears[id] = 1;
	}
	
	const std::string path2 = "sprites/ui/slider.png";
	auto b = cam.norm_to_pixels(SLIDER_AXIS_BOUNDS);
	std::pair<float, float> axisBounds = { b.x, b.y };
	slider().setup(SLIDER_POS, SLIDER_SIZE, cam, TextureManager::t_slider);
	slider().setup_slider(false, axisBounds);
	slider().set_anchored_btns(btns);

	for (int i = 0; i < 10; i++) {
		//slots[i].set_unit(-1, 1);
		sf::Vector2f scale = cam.get_norm_sprite_scale(slots[i].sprite, SLOT_SCALE);
		slots[i].sprite.setScale(scale);
		slots[i].sprite.setOrigin(slots[i].sprite.getLocalBounds().size * 0.5f);
	}

	returnBtn().setup(RETURN_BTN_POS, RETURN_BTN_SIZE, cam, TextureManager::t_returnBtn);
}
void ArmoryMenu::set_up_buttons() {
	/*
	* Will add the set ups later
	*/
}
void ArmoryMenu::reset_positions() {
	sf::Vector2f pos = cam.norm_to_pixels(FIRST_SLOT_POS);
	sf::Vector2f inc = cam.norm_to_pixels(SLOT_INCREMENT);
	float startX = pos.x;

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 5; j++) {
			int k = i * 5 + j;
			slots[k].set_pos(pos);
			pos.x += inc.x;
		}
		pos.x = startX;
		pos.y += inc.y;
	}

	pos = cam.norm_to_pixels(SLIDER_SLOT_POS);
	inc = cam.norm_to_pixels(SLIDER_SLOT_INCREMENT);

	for (int i = 0; i < 3; i++) {
		buttonManager.buttons[i].set_pos(pos);
		pos.x += inc.x;
	}

	slider().set_norm_pos(SLIDER_POS, cam);
	slider().reset_offsets();

	returnBtn().set_norm_pos(RETURN_BTN_POS, cam);
}

void ArmoryMenu::update(float dt) {
	check_mouse_hover();

	slider().update(cam.getMouseScreenPos(), dt);
}

void ArmoryMenu::draw() {
	if (paused && inStageMode) stageSetMenu.draw();
	
	// Sraw Unit slots from the SLECTOR
	for (int i = 0; i < TOTAL_PLAYER_UNITS; i++) {
		sf::Sprite& slotSprite = unitSelectionBtn(i).sprite;
		sf::FloatRect rect = slotSprite.getGlobalBounds();

		cam.queue_world_draw(&slotSprite, rect);
		if (unit_is_unusable(i)) cam.draw_overlay(slotSprite, RED_TRANSPARENT);
	}
	cam.queue_ui_draw(&slider().sprite);

	buttonManager.draw(cam, TOTAL_PLAYER_UNITS); // Draw all other buttons

	for (int i = 0; i < ARMORY_SLOTS; i++)
		cam.queue_ui_draw(&slots[i].sprite);

	if (dragging_unit()) cam.queue_ui_draw(&cam.cursor.ui);
}

bool ArmoryMenu::on_mouse_press(bool isM1) {
	if (paused && inStageMode) return stageSetMenu.on_mouse_press(isM1);
	if (dragging_unit()) return false;

	const auto& mPos = cam.getMouseScreenPos();
	if (slider().check_mouse_hover(mPos))
		slider().onClick(isM1);

	buttonManager.on_mouse_press(mPos, isM1);

	if (isM1) {
		// Dont need the UNSUABLE check here. Unsuable untis will already be unequipped
		for (int i = 0; i < ARMORY_SLOTS; i++) {
			ArmorySlot& slot = slots[i];
			if (!slot.empty() && slot.hovered_over(mPos)) {
				int id = slot.id;
				slot.clear();
				shift_empty_slots();
				start_dragging_unit(id);
				break;
			}
		}
	}

	return true;
}
bool ArmoryMenu::on_mouse_release(bool isM1) {
	if (paused && inStageMode) return stageSetMenu.on_mouse_press(isM1);

	if (isM1) {
		slider().set_mouse_hold(false);
		
		if (dragging_unit())
			drag_unit_into_slot();

		release_hold();
	}

	curHeldUnit = { -1, -1 };
	return true;
}
void ArmoryMenu::check_mouse_hover() {
	if (paused && inStageMode) {
		stageSetMenu.check_mouse_hover();
		return;
	}

	sf::Vector2i mPos = cam.getMouseScreenPos();

	slider().check_mouse_hover(mPos);
	buttonManager.check_mouse_hover(mPos);
}

#pragma region Unit Slots and Dragging
void ArmoryMenu::drag_unit_into_slot() {
	auto mPos = cam.getMouseScreenPos();
	for (int i = 0; i < ARMORY_SLOTS; i++) {
		if (slots[i].hovered_over(mPos)) {
			slots[i].set_unit(curHeldUnit);
			shift_empty_slots();
			return;
		}
	}
}
void ArmoryMenu::start_dragging_unit(int id) {
	if (unit_is_slotted(id) || unit_is_unusable(id)) return;

	curHeldUnit = { id, unitSelectionGears[id] };
	cam.set_cursor_ui(TextureManager::getUnitSlot(curHeldUnit), DRAG_SLOT_ORIGIN, DRAG_SLOT_OPACITY, SLOT_SCALE);
}
void ArmoryMenu::shift_empty_slots() {
	filledUnitSlots = 0;

	for (int read = 0, write = 0; read < ARMORY_SLOTS; read++) {
		if (!slots[read].empty()) {
			if (write != read) {
				slots[write].set_unit(slots[read].slotted_unit());
				slots[read].clear();  
			}
			write++;
			filledUnitSlots++;
		}
	}
}
void ArmoryMenu::update_selection_slot(int id, int gear) {
	if (id >= 100) return;

	for (int i = 0; i < filledUnitSlots; i++)
		if (slots[i].id == id) {
			slots[i].set_unit(id, gear);
			break;
		}

	unitSelectionGears[id] = gear;
	std::cout << "gear: " << gear << std::endl;
	unitSelectionBtn(id).set_texture(TextureManager::getUnitSlot(id, gear));
}
void ArmoryMenu::remove_unusable_units() {
	for (int i = 0; i < ARMORY_SLOTS; i++) {
		ArmorySlot& slot = slots[i];
		if (!slot.empty() && unit_is_unusable(slot.id)) 
			slot.clear();
	}

	shift_empty_slots();
}
#pragma endregion

#pragma Armory Slot
void ArmorySlot::set_unit(int ID, int g, int c) {
	id = ID; gear = g; core = c;

	sprite.setTexture(TextureManager::getUnitSlot(id, gear), true);
}
void ArmorySlot::set_unit(std::pair<int, int> unit) {
	set_unit(unit.first, unit.second);
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

	slots[0].set_unit(0, 3, 0);
	slots[1].set_unit(1, 2);

	return slots;
}
#pragma endregion