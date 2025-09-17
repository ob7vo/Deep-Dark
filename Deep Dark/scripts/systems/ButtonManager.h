#pragma once
#include "Button.h"
struct ButtonManager
{
	std::vector<Button> buttons;

	ButtonManager();

	void tick(sf::RenderWindow& window, sf::Vector2f mousePos);
	void draw(sf::RenderWindow& window);
	void check_mouse_hover(sf::Vector2f mousePos);
	void register_click(sf::Vector2f mousePos);

	inline void add_button(sf::Vector2f pos, sf::Vector2f bounds, sf::Color color, std::function<void()> onClick = {}){
		buttons.emplace_back(pos, bounds, color, onClick);
	}
};

