#include "pch.h"
#include "Menu.h"
#include "ButtonManager.h"
#include "Button.h"
#include "Camera.h"

std::string MenuBase::GetMenuName(MenuType type) {
	switch (type) {
	case MenuType::MAIN_MENU: return "[MAIN_MENU]";
	case MenuType::HOME_BASE: return "[HOME_BASE]";
	case MenuType::STAGE_SELECT: return "[STAGE_SELECT]";
	case MenuType::STAGE_SET: return "[STAGE_SET]";
	case MenuType::ARMORY_EQUIP: return "[ARMORY_EQUIP]";
	case MenuType::WORKSHOP_MENU: return "[WORKSHOP_MENU]";
	default: return "[NO_MENU?]";
	}
}

template<int BUTTONS>
void Menu<BUTTONS>::draw() { if (this->visible) this->buttonManager.draw(this->cam); }
template<int BUTTONS>
bool ButtonMenu<BUTTONS>::on_mouse_press(bool isM1) {
	if (!this->clickable) return false;

	return this->buttonManager.on_mouse_press(this->cam.getMouseScreenPos(), isM1);
}
template<int BUTTONS>
void ButtonMenu<BUTTONS>::on_mouse_hold(bool isM1) { return; }
template<int BUTTONS>
void ButtonMenu<BUTTONS>::check_mouse_hover()  {
	this->buttonManager.check_mouse_hover(this->cam.getMouseScreenPos());
}

template<int BUTTONS>
void ButtonManager<BUTTONS>::draw(Camera& cam, int start, int end) {
	for (int i = start; i < end; i++)
		if (buttons[i].visible)
			cam.renderer.queue_ui_draw(&buttons[i].sprite);
}
template<int BUTTONS>
void ButtonManager<BUTTONS>::check_mouse_hover(sf::Vector2i mPos, int start, int end) {
	for (int i = start; i < end; i++)
		if (buttons[i].check_mouse_hover(mPos)) return;
}
template<int BUTTONS>
bool ButtonManager<BUTTONS>::on_mouse_press(sf::Vector2i mPos, bool m1, int start, int end) {
	for (int i = start; i < end; i++)
		if (buttons[i].visible && buttons[i].onClick && buttons[i].mouseIsHovering(mPos)) {
			buttons[i].onClick(m1);
			return true;
		}

	return false;
}
template<int BUTTONS>
void ButtonManager<BUTTONS>::translate(sf::Vector2f delta, int start, int end) {
	for (int i = start; i < end; i++)
		buttons[i].translate(delta);
}

template struct ButtonManager<UI::ArmoryMenu::BTN_COUNT>; // 9
template struct ButtonManager<UI::StageSelect::BTN_COUNT>; // 4
template struct ButtonManager<UI::StageSelect::StageNode::BTN_COUNT>; // 6
template struct ButtonManager<UI::ArmoryMenu::StagePreview::BTN_COUNT>; // 3
//template struct ButtonManager<UI::StageUI::BTN_COUNT>; // 3 DUPE
template struct ButtonManager<UI::StageUI::PauseMenu::BTN_COUNT>; // 2
//template struct ButtonManager<UI::StageUI::ResultsScreen::BTN_COUNT>; // 3 DUPE
//template struct ButtonManager<UI::StartMenu::BTN_COUNT>; // 3 DUPE
template struct ButtonManager<UI::Workshop::BTN_COUNT>; // 15

template struct Menu<UI::ArmoryMenu::BTN_COUNT>;
template struct Menu<UI::StageSelect::BTN_COUNT>;
template struct Menu<UI::StageSelect::StageNode::BTN_COUNT>;
template struct Menu<UI::ArmoryMenu::StagePreview::BTN_COUNT>;
//template struct Menu<UI::StageUI::BTN_COUNT>;
template struct Menu<UI::StageUI::PauseMenu::BTN_COUNT>;
//template struct Menu<UI::StageUI::ResultsScreen::BTN_COUNT>;
//template struct Menu<UI::StartMenu::BTN_COUNT>;
template struct Menu<UI::Workshop::BTN_COUNT>;

template struct ButtonMenu<UI::ArmoryMenu::BTN_COUNT>;
template struct ButtonMenu<UI::StageSelect::BTN_COUNT>;
template struct ButtonMenu<UI::StageSelect::StageNode::BTN_COUNT>;
template struct ButtonMenu<UI::ArmoryMenu::StagePreview::BTN_COUNT>;
//template struct ButtonMenu<UI::StageUI::BTN_COUNT>;
template struct ButtonMenu<UI::StageUI::PauseMenu::BTN_COUNT>;
//template struct ButtonMenu<UI::StageUI::ResultsScreen::BTN_COUNT>;
//template struct ButtonMenu<UI::StartMenu::BTN_COUNT>;
template struct ButtonMenu<UI::Workshop::BTN_COUNT>;
