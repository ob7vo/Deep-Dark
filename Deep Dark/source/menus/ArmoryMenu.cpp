#include "pch.h"
#include "ArmoryMenu.h"
#include "UILayout.h"
#include "ArmorySlot.h"
#include "Camera.h"
#include "WorkshopMenu.h"
#include "UITextures.h"
#include "UnitSaveData.h"

using namespace UI::ArmoryMenu;
using namespace UI::Colors;
using namespace UnitConfig;

ArmoryMenu::ArmoryMenu(Camera& cam) : Menu(cam), stagePreviewMenu(cam) {
	// Inventory of Units
	for (int id = 0; id < TOTAL_PLAYER_UNITS; id++) {
		inventorySlotBtn(id).setup(UI::ZERO, INVENTORY_SLOT_SCALE, Textures::UI::getUnitSlot(id, 1));
	}

	// Slider
	inventorySlider.setup(SLIDER_POS, SLIDER_SIZE, Textures::UI::t_slider);
	inventorySlider.setup_slider(false, Screen::toPixels(SLIDER_RANGE));

	// Equip/Armory Slots
	for (int i = 0; i < EQUIP_SLOTS; i++) {
		equipSlots[i].set_unit(-1, 1, true);

		sf::Vector2f scale = Screen::getSpriteScale(equipSlots[i].sprite, EQUIP_SLOT_SCALE);
		equipSlots[i].sprite.setScale(scale);

		equipSlots[i].sprite.setOrigin(equipSlots[i].sprite.getLocalBounds().size * 0.5f);
	}
	equipSlots = ArmorySlot::default_armory_loadout(cam);
	filledUnitSlots = std::count_if(equipSlots.begin(), equipSlots.end(),
		[](const ArmorySlot& slot) { return slot.id != -1; });

	// Other Buttons
	returnBtn().setup(RETURN_BTN_POS, RETURN_BTN_SIZE, Textures::UI::t_returnBtn);
	openStageSetBtn().setup(STAGE_SET_BTN_POS, STAGE_SET_BTN_SIZE, Textures::UI::t_enterStageSetBtn);
	openStageSetBtn().onClick = [this](bool m1) {if (m1) mode = Mode::ViewingStagePreview; };

	stagePreviewMenu.closeBtn().onClick = [this](bool m1) {
		if (m1) mode = Mode::StagePreparation;
		};
}
void ArmoryMenu::on_enter() {
	openStageSetBtn().visible = (mode == Mode::StagePreparation);

	// Re-setup the buttons for the slider
	inventorySlider.get_buttons().clear();
	std::vector<Button*> newSliderBtns = {};
	for (int id = 0; id < TOTAL_PLAYER_UNITS; id++) {
		displayedGears[id] = UnitSaveData::GetMaxGear(id);
		inventorySlotBtn(id).visible = unitIsOwned(id);

		if (unitIsOwned(id)) {
			inventorySlotBtn(id).set_texture(Textures::UI::getUnitSlot(id, displayedGears[id]));
			newSliderBtns.push_back(&inventorySlotBtn(id));
		}
	}
	inventorySlider.set_anchored_btns(newSliderBtns);

	reset_positions();
}
void ArmoryMenu::reset_positions() {
	sf::Vector2f equippedUnitPos = Screen::toPixels(FIRST_EQUIP_SLOT_POS);
	sf::Vector2f equippedUnitPosIncrement = Screen::toPixels(EQUIP_SLOT_SPACING);
	float startX = equippedUnitPos.x;

	int columns = UnitConfig::MAX_EQUIP_SLOTS * 0.5f;

	for (int row = 0; row < 2; row++) {
		for (int col = 0; col < columns; col++) {
			equipSlots[row * columns + col].set_pos(equippedUnitPos);
			equippedUnitPos.x += equippedUnitPosIncrement.x;
		}
		equippedUnitPos.x = startX;
		equippedUnitPos.y += equippedUnitPosIncrement.y;
	}

	sf::Vector2f unitSelectionBtnPos = Screen::toPixels(FIRST_INVENTORY_SLOT_POS);
	float unitBtnSelectionPosIncrement = Screen::toPixels(INVENTORY_SLOT_SPACING).x;

	for (int id = 0; id < TOTAL_PLAYER_UNITS; id++) {
		if (!unitIsOwned(id)) continue;

		inventorySlotBtn(id).set_pos(unitSelectionBtnPos);
		unitSelectionBtnPos.x += unitBtnSelectionPosIncrement;
	}

	inventorySlider.set_pos(Screen::toPixels(SLIDER_POS));
	inventorySlider.reset_offsets();
	returnBtn().set_pos(Screen::toPixels(RETURN_BTN_POS));
	openStageSetBtn().set_pos(Screen::toPixels(STAGE_SET_BTN_POS));

	stagePreviewMenu.reset_positions();
}

void ArmoryMenu::update(float dt) {
	check_mouse_hover();

	inventorySlider.update(cam.getMouseScreenPos(), dt);
}
void ArmoryMenu::draw() {	
	// Draw the Unit slots from the inventory
	for (int id = 0; id < TOTAL_PLAYER_UNITS; id++) {
		if (!unitIsOwned(id)) continue;
		Button& unitBtn = inventorySlotBtn(id);

		// The global Bounds are fro checking if the sprite is within the camera
		cam.renderer.queue_world_draw(&unitBtn.sprite, unitBtn.sprite.getGlobalBounds());
		if (!equipSlots[id].usable) cam.renderer.draw_overlay(unitBtn.sprite, RED_TRANSPARENT);
	}

	// Checking the rest of the buttons from the last inventory button
	int from = TOTAL_PLAYER_UNITS;
	buttonManager.draw(cam, from);

	// Draw the slider
	cam.renderer.queue_ui_draw(&inventorySlider.sprite);

	// Draw the slots of the current equipped units
	for (int i = 0; i < EQUIP_SLOTS; i++)
		cam.renderer.queue_ui_draw(&equipSlots[i].sprite);

	// Draw any dragged units
	if (dragging_unit()) cam.renderer.draw_cursor_ui();

	// Draw the StageSetMenu last (if present) to overlay everything
	if (mode == Mode::ViewingStagePreview) stagePreviewMenu.draw();
}

#pragma region Mouse
bool ArmoryMenu::check_mouse_press_for_equip_slots(bool isM1, sf::Vector2i mouseScreenPos) {
	// The Armory slots can be clicked and right clicked
	// Checking if a Unit is unusable is unnecessary, they can't be equipped otherwise

	// LEFT CLICK
	if (isM1) 
	{
		for (int i = 0; i < EQUIP_SLOTS; i++) 
		{
			if (!equipSlots[i].empty() && equipSlots[i].hovered_over(mouseScreenPos)) 
			{
				equipSlots[i].clear();
				shift_empty_slots();
				start_dragging_unit(equipSlots[i].id);

				return true;
			}
		}
	}
	// RIGHT CLICK
	else {
		for (int i = 0; i < EQUIP_SLOTS; i++) 
			if (!equipSlots[i].empty() && equipSlots[i].hovered_over(mouseScreenPos)) 
				inventorySlotBtn(equipSlots[i].id).onClick(false);
	}

	return false;
}
bool ArmoryMenu::on_mouse_press(bool isM1) {
	const auto& mScreenPos = cam.getMouseScreenPos();

	if (mode == Mode::ViewingStagePreview) return stagePreviewMenu.on_mouse_press(isM1);
	if (dragging_unit()) return false;

	if (inventorySlider.try_mouse_press(mScreenPos, isM1)) return true;
	if (buttonManager.on_mouse_press(mScreenPos, isM1)) return true;

	
	if (check_mouse_press_for_equip_slots(isM1, mScreenPos)) return true;

	return false;
}
bool ArmoryMenu::on_mouse_release(bool isM1) {
	// For letting go of the slider or dragged Unit
	if (isM1) {
		inventorySlider.set_mouse_hold(false);
		
		if (dragging_unit())
			drag_unit_into_slot();
	}

	release_held_unit();
	return true;
}
void ArmoryMenu::check_mouse_hover() {
	sf::Vector2i mScreenPos = cam.getMouseScreenPos();

	if (mode == Mode::ViewingStagePreview) {
		stagePreviewMenu.check_mouse_hover();
		return;
	}

	// Sliders aren't part of the Button array
	inventorySlider.check_mouse_hover(mScreenPos);

	// Checking the rest of the buttons
	buttonManager.check_mouse_hover(mScreenPos);
}
#pragma endregion

#pragma region Unit Slots and Dragging
void ArmoryMenu::drag_unit_into_slot() {
	auto mPos = cam.getMouseScreenPos();

	for (int i = 0; i < EQUIP_SLOTS; i++) {
		if (equipSlots[i].hovered_over(mPos)) {
			equipSlots[i].set_unit(curHeldUnit, !unit_is_unusable(curHeldUnit));
			shift_empty_slots();

			return;
		}
	}
}
void ArmoryMenu::start_dragging_unit(int id) {
	if (unit_is_equipped(id)) {
		std::cout << "Equipped" << std::endl;
		return;
	}
	if (unit_is_unusable(id, displayedGears[id])) {
		std::cout << "unusable" << std::endl;
		return;
	}

	curHeldUnit = { id, displayedGears[id] };
	cam.set_cursor_ui(Textures::UI::getUnitSlot(curHeldUnit), DRAG_SLOT_ORIGIN, DRAG_SLOT_OPACITY, INVENTORY_SLOT_SCALE);
}
void ArmoryMenu::shift_empty_slots() {
	filledUnitSlots = 0;

	for (int read = 0, write = 0; read < EQUIP_SLOTS; read++) {
		if (!equipSlots[read].empty()) {
			if (write != read) {
				equipSlots[write].copy(equipSlots[read]);
				equipSlots[read].clear();
			}

			write++;
			filledUnitSlots++;
		}
	}
}
void ArmoryMenu::update_display_of_unit(int id, int gear) {
	if (id >= UnitConfig::ENEMY_ID_OFFSET) return; // IDs >= 100 belong to enemies

	// Change the display within the equipped Units
	for (int i = 0; i < filledUnitSlots; i++)
		if (equipSlots[i].id == id) {
			// Check if the unit is usable again, as specific gears might not be
			equipSlots[i].set_unit(id, gear, !unit_is_unusable(id, gear));
			break;
		}

	// Change the display within the selection
	displayedGears[id] = gear;
	inventorySlotBtn(id).set_texture(Textures::UI::getUnitSlot(id, gear));
}

#pragma endregion
