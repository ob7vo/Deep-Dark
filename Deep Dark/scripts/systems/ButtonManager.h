#pragma once
#include "Button.h"

template<int BUTTONS>
struct ButtonManager
{
	std::array<Button, BUTTONS> buttons;
	ButtonManager() = default;

	void draw(Camera& cam, int start = 0, int end = BUTTONS) {
		for (int i = start; i < end; i++)
			cam.queue_ui_draw(&buttons[i].sprite);
	}
	inline void check_mouse_hover(sf::Vector2i mPos, int start = 0, int end = BUTTONS) {
		for (int i = start; i < end; i++)
			if (buttons[i].check_mouse_hover(mPos)) return;
	}
	inline void register_click(sf::Vector2i mPos, int start = 0, int end = BUTTONS) {
		for (int i = start; i < end; i++)
			if (buttons[i].is_hovering(mPos) && buttons[i].onClick) {
				buttons[i].onClick();
				return;
			}
	}


	inline void add_button(sf::Vector2f pos, sf::Vector2f bounds, sf::Color color, std::function<void()> onClick = {}){
		buttons.emplace_back(pos, bounds, color, onClick);
	}
};

