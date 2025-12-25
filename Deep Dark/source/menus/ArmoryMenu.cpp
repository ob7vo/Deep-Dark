#include "pch.h"
#include "ArmoryMenu.h"
#include "UnitData.h"
#include "UILayout.h"
#include "ArmorySlot.h"
#include "Camera.h"
#include "WorkshopMenu.h"
#include "UITextures.h"

using namespace UI::ArmoryMenu;
using namespace UI::Colors;
using namespace UnitData;

ArmoryMenu::ArmoryMenu(Camera& cam) : Menu(cam), stageSetMenu(cam) {
	std::vector<Button*> btns = {};
	for (int id = 0; id < 3; id++) {
		Button& btn = buttonManager.buttons[id];
		btns.push_back(&btn);

		btn.setup(UI::ZERO, SLIDER_SLOT_SCLAE, cam, Textures::UI::getUnitSlot(id, 1));
		displayedGears[id] = 1;
	}

	const std::string path2 = "sprites/ui/slider.png";
	auto bounds = cam.norm_to_pixels(SLIDER_AXIS_BOUNDS);
	std::pair<float, float> axisBounds = { bounds.x, bounds.y };
	slider().setup(SLIDER_POS, SLIDER_SIZE, cam, Textures::UI::t_slider);
	slider().setup_slider(false, axisBounds);
	slider().set_anchored_btns(btns);

	for (int i = 0; i < 10; i++) {
		slots[i].set_unit(-1, 1, true);

		sf::Vector2f scale = cam.get_norm_sprite_scale(slots[i].sprite, SLOT_SCALE);
		slots[i].sprite.setScale(scale);

		slots[i].sprite.setOrigin(slots[i].sprite.getLocalBounds().size * 0.5f);
	}

	returnBtn().setup(RETURN_BTN_POS, RETURN_BTN_SIZE, cam, Textures::UI::t_returnBtn);
	openStageSetBtn().setup(STAGE_SET_BTN_POS, STAGE_SET_BTN_SIZE, cam, Textures::UI::t_enterStageSetBtn);
	openStageSetBtn().onClick = [this](bool m1) {if (m1) paused = true; };

	stageSetMenu.closeBtn().onClick = [this](bool m1) {
		if (m1) paused = false;
		};
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

	stageSetMenu.reset_positions();
}

void ArmoryMenu::update(float dt) {
	check_mouse_hover();

	slider().update(cam.getMouseScreenPos(), dt);
}

void ArmoryMenu::draw() {	
	// Draw the Unit slots from the SELECTOR
	for (int i = 0; i < TOTAL_PLAYER_UNITS; i++) {
		sf::Sprite& slotSprite = unitSelectionBtn(i).sprite;
		sf::FloatRect rect = slotSprite.getGlobalBounds();

		cam.queue_world_draw(&slotSprite, rect);
		if (!slots[i].usauble) cam.draw_overlay(slotSprite, RED_TRANSPARENT);
	}

	// Draw the slider
	cam.queue_ui_draw(&slider().sprite);

	// Checking the rest of the buttons (EXCLUDING the openStageSetBtn)
	buttonManager.draw(cam, TOTAL_PLAYER_UNITS, BTN_COUNT - 1); 

	// Draw the slots of the current equpied units
	for (int i = 0; i < ARMORY_SLOTS; i++)
		cam.queue_ui_draw(&slots[i].sprite);

	if (dragging_unit()) cam.queue_ui_draw(&cam.cursor.ui);

	if (inStageMode) {
		cam.queue_ui_draw(&openStageSetBtn().sprite);
		if (paused) stageSetMenu.draw();
	}
}

bool ArmoryMenu::on_mouse_press(bool isM1) {
	const auto& mScreenPos = cam.getMouseScreenPos();

	if (inStageMode) {
		if (paused) return stageSetMenu.on_mouse_press(isM1);
		else return openStageSetBtn().try_mouse_press(mScreenPos, isM1);
	}
	if (dragging_unit()) return false;

	if (slider().try_mouse_press(mScreenPos, isM1)) return true;

	// Checking the rest of the buttons (EXCLUDING the openStageSetBtn)
	if (buttonManager.on_mouse_press(mScreenPos, isM1, BTN_COUNT - 1)) return true;

	if (isM1) {
		// Dont need the UNSUABLE check here. Unsuable untis will already be unequipped
		for (int i = 0; i < ARMORY_SLOTS; i++) {
			ArmorySlot& slot = slots[i];
			if (!slot.empty() && slot.hovered_over(mScreenPos)) {
				int id = slot.id;

				slot.clear();
				shift_empty_slots();

				start_dragging_unit(id);
				return true;
			}
		}
	}

	return false;
}
bool ArmoryMenu::on_mouse_release(bool isM1) {
	// For letting go of the slider or dragged Unit
	if (isM1) {
		slider().set_mouse_hold(false);
		
		if (dragging_unit())
			drag_unit_into_slot();
	}

	release_held_unit();
	return true;
}

void ArmoryMenu::check_mouse_hover() {
	sf::Vector2i mScreenPos = cam.getMouseScreenPos();

	if (inStageMode) {
		if (paused) {
			stageSetMenu.check_mouse_hover();
			return;
		}
		else openStageSetBtn().check_mouse_hover(mScreenPos);
	}

	// Sliders aren't part of the Button array
	slider().check_mouse_hover(mScreenPos);

	// Checking the rest of the buttons (EXCLUDING the openStageSetBtn)
	buttonManager.check_mouse_hover(mScreenPos, BTN_COUNT - 1);
}

#pragma region Unit Slots and Dragging
void ArmoryMenu::drag_unit_into_slot() {
	auto mPos = cam.getMouseScreenPos();

	for (int i = 0; i < ARMORY_SLOTS; i++) {
		if (slots[i].hovered_over(mPos)) {
			slots[i].set_unit(curHeldUnit, !unit_is_unusable(curHeldUnit));
			shift_empty_slots();

			return;
		}
	}
}
void ArmoryMenu::start_dragging_unit(int id) {
	if (unit_is_equipped(id) || unit_is_unusable(id, displayedGears[id])) return;

	curHeldUnit = { id, displayedGears[id] };
	cam.set_cursor_ui(Textures::UI::getUnitSlot(curHeldUnit), DRAG_SLOT_ORIGIN, DRAG_SLOT_OPACITY, SLOT_SCALE);
}
void ArmoryMenu::shift_empty_slots() {
	filledUnitSlots = 0;

	for (int read = 0, write = 0; read < ARMORY_SLOTS; read++) {
		if (!slots[read].empty()) {
			if (write != read) {
				slots[write].set_unit(slots[read].id, slots[read].gear, slots[read].usauble);
				slots[read].clear();  
			}
			write++;
			filledUnitSlots++;
		}
	}
}
void ArmoryMenu::change_displayed_gear(int id, int gear) {
	if (id >= UnitData::ENEMY_ID_OFFSET) return; // IDs >= 100 belong to enemies

	for (int i = 0; i < filledUnitSlots; i++)
		if (slots[i].id == id) {
			// Check if the unit is usauble again, as specific gears might not be
			slots[i].set_unit(id, gear, !unit_is_unusable(id, gear));
			break;
		}

	displayedGears[id] = gear;
	unitSelectionBtn(id).set_texture(Textures::UI::getUnitSlot(id, gear));
}

#pragma endregion
