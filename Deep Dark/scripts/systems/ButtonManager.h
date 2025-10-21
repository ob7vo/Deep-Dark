#pragma once
#include "Button.h"

template<int BUTTONS>
struct ButtonManager
{
	std::array<Button, BUTTONS> buttons;
	ButtonManager() = default;

	void draw(Camera& cam) {
		for (int i = 0; i < BUTTONS; i++)
			cam.queue_ui_draw(&buttons[i].sprite);
	}
	void check_mouse_hover(sf::Vector2f mousePos, int start = 0, int end = BUTTONS) {
		for (int i = start; i < end; i++)
			buttons[i].check_mouse_hover(mousePos);
	}
	void register_click(sf::Vector2f mousePos, int start = 0, int end = BUTTONS) {
		for (int i = start; i < end; i++)
			if (buttons[i].is_hovering(mousePos) && buttons[i].onClick) {
				buttons[i].onClick();
				return;
			}
	}
	void tick(sf::RenderWindow& window, sf::Vector2f mousePos) {
		draw(window);
		check_mouse_hover(mousePos);
	}

	inline void add_button(sf::Vector2f pos, sf::Vector2f bounds, sf::Color color, std::function<void()> onClick = {}){
		buttons.emplace_back(pos, bounds, color, onClick);
	}
};

