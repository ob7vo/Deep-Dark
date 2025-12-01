#include "pch.h"
#include "Menu.h"
#include "ButtonManager.h"
#include "Camera.h"

template<int BUTTONS>
void Menu<BUTTONS>::draw() { buttonManager.draw(cam); }
template<int BUTTONS>
bool Menu<BUTTONS>::on_mouse_press(bool isM1) {
	return buttonManager.on_mouse_press(cam.getMouseScreenPos(), isM1);
}
template<int BUTTONS>
void Menu<BUTTONS>::check_mouse_hover()  {
	buttonManager.check_mouse_hover(cam.getMouseScreenPos());
}

template<int BUTTONS>
void ButtonManager<BUTTONS>::draw(Camera& cam, int start, int end) {
	for (int i = start; i < end; i++)
		cam.queue_ui_draw(&buttons[i].sprite);
}
template<int BUTTONS>
void ButtonManager<BUTTONS>::check_mouse_hover(sf::Vector2i mPos, int start, int end) {
	for (int i = start; i < end; i++)
		if (buttons[i].check_mouse_hover(mPos)) return;
}
template<int BUTTONS>
bool ButtonManager<BUTTONS>::on_mouse_press(sf::Vector2i mPos, bool m1, int start, int end) {
	for (int i = start; i < end; i++)
		if (buttons[i].is_hovering(mPos) && buttons[i].onClick) {
			buttons[i].onClick(m1);
			return true;
		}

	return false;
}


template class ButtonManager<UI::ArmoryMenu::BTN_COUNT>;
template class ButtonManager<UI::StageSelect::BTN_COUNT>;
template class ButtonManager<UI::StageSet::BTN_COUNT>;
template class ButtonManager<UI::StageUI::UI_BTN_COUNT>;
template class ButtonManager<UI::StageUI::PAUSE_MENU_BTN_COUNT>;
template class ButtonManager<UI::StartMenu::BTN_COUNT>;
template class ButtonManager<UI::Workshop::BTN_COUNT>;

template class Menu<UI::ArmoryMenu::BTN_COUNT>;
template class Menu<UI::StageSelect::BTN_COUNT>;
template class Menu<UI::StageSet::BTN_COUNT>;
template class Menu<UI::StageUI::UI_BTN_COUNT>;
template class Menu<UI::StageUI::PAUSE_MENU_BTN_COUNT>;
template class Menu<UI::StartMenu::BTN_COUNT>;
template class Menu<UI::Workshop::BTN_COUNT>;
