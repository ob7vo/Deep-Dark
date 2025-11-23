#include "ArmoryMenu.h"
#include "UnitData.h"
#include "UILayout.h"
#include "ArmorySlot.h"
#include "WorkshopMenu.h"

using namespace UI::ArmoryMenu;
using namespace UnitData;

ArmoryMenu::ArmoryMenu(Camera& cam) : Menu(cam) {
	slots = ArmorySlot::default_armory_loadout();

	std::vector<Button*> btns = {};
	for (int i = 0; i < 3; i++) {
		const std::string path = get_unit_folder_path(i, 0) + "slot.png";
		Button& btn = buttonManager.buttons[i];
		btns.push_back(&btn);

		btn.set_ui_params(UI::ZERO, SLIDER_SLOT_SCLAE, path, cam);
		unitSelectionForms[i] = 1;
	}
	
	const std::string path2 = "sprites/ui/slider.png";
	auto b = cam.norm_to_pixels(SLIDER_AXIS_BOUNDS);
	std::pair<float, float> axisBounds = { b.x, b.y };
	slider().set_ui_params(SLIDER_POS, SLIDER_SIZE, path2, cam);
	slider().setup(false, axisBounds);
	slider().set_anchored_btns(btns);

	for (int i = 0; i < 10; i++) {
		slots[i].set_unit(-1, -1);
		sf::Vector2f scale = cam.get_norm_sprite_scale(slots[i].sprite, SLOT_SCALE);
		slots[i].sprite.setScale(scale);
		slots[i].sprite.setOrigin(slots[i].sprite.getLocalBounds().size * 0.5f);
	}

	std::string texPath3 = "sprites/ui/return.png";
	returnBtn().set_ui_params(RETURN_BTN_POS, RETURN_BTN_SIZE, texPath3, cam);	
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
	for (int i = 0; i < TOTAL_PLAYER_UNITS; i++) {
		sf::FloatRect rect = unitSelectionBtn(i).sprite.getGlobalBounds();
		cam.queue_world_draw(&unitSelectionBtn(i).sprite, rect);
	}
	cam.queue_ui_draw(&slider().sprite);
	buttonManager.draw(cam, TOTAL_PLAYER_UNITS);

	for (int i = 0; i < ARMORY_SLOTS; i++)
		cam.queue_ui_draw(&slots[i].sprite);

	if (dragging_unit()) cam.queue_ui_draw(&cam.cursor.ui);
}

bool ArmoryMenu::on_mouse_press(bool isM1) {
	if (dragging_unit()) return false;

	auto& mPos = cam.getMouseScreenPos();
	if (slider().check_mouse_hover(mPos))
		slider().onClick(isM1);

	buttonManager.on_mouse_press(mPos, isM1);

	if (isM1) {
		for (int i = 0; i < ARMORY_SLOTS; i++) {
			ArmorySlot& slot = slots[i];
			if (!slot.empty() && slot.hovered_over(mPos)) {
				int id = slot.id;
				slot.clear();
				shift_empty_slots();
				drag_unit(id);
				break;
			}
		}
	}

	return true;
}
bool ArmoryMenu::on_mouse_release(bool isM1) {
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
	sf::Vector2i mPos = cam.getMouseScreenPos();

	slider().check_mouse_hover(mPos);
	buttonManager.check_mouse_hover(mPos);
}

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
void ArmoryMenu::drag_unit(int id) {
	if (unit_is_slotted(id)) return;

	curHeldUnit = { id, unitSelectionForms[id] };

	sf::Texture tex = get_slot_texture(curHeldUnit);

	cam.set_cursor_ui(tex, DRAG_SLOT_ORIGIN, DRAG_SLOT_OPACITY, SLOT_SCALE);
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

void ArmorySlot::set_unit(int ID, int g, int c) {
	id = ID; gear = g; core = c;

	texture = UnitData::get_slot_texture(id, gear);
	sprite.setTexture(texture, true);
}