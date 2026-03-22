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
void Menu<BUTTONS>::draw() { buttonManager.draw(cam); }
template<int BUTTONS>
bool Menu<BUTTONS>::on_mouse_press(bool isM1) {
	if (!clickable) return false;

	return buttonManager.on_mouse_press(cam.getMouseScreenPos(), isM1);
}
template<int BUTTONS>
void Menu<BUTTONS>::check_mouse_hover()  {
	buttonManager.check_mouse_hover(cam.getMouseScreenPos());
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


template struct ButtonManager<UI::ArmoryMenu::BTN_COUNT>;
template struct ButtonManager<UI::StageSelect::BTN_COUNT>;
template struct ButtonManager<UI::StageSelect::StageNode::BTN_COUNT>;
template struct ButtonManager<UI::ArmoryMenu::StagePreview::BTN_COUNT>;
template struct ButtonManager<UI::StageUI::BTN_COUNT>;
template struct ButtonManager<UI::StageUI::PauseMenu::BTN_COUNT>;
template struct ButtonManager<UI::StageUI::ResultsScreen::BTN_COUNT>;
template struct ButtonManager<UI::StartMenu::BTN_COUNT>;
template struct ButtonManager<UI::Workshop::BTN_COUNT>;

template struct Menu<UI::ArmoryMenu::BTN_COUNT>;
template struct Menu<UI::StageSelect::BTN_COUNT>;
template struct Menu<UI::StageSelect::StageNode::BTN_COUNT>;
template struct Menu<UI::ArmoryMenu::StagePreview::BTN_COUNT>;
template struct Menu<UI::StageUI::BTN_COUNT>;
template struct Menu<UI::StageUI::PauseMenu::BTN_COUNT>;
template struct Menu<UI::StageUI::ResultsScreen::BTN_COUNT>;
template struct Menu<UI::StartMenu::BTN_COUNT>;
template struct Menu<UI::Workshop::BTN_COUNT>;
