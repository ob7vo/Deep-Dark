#include "Button.h"
#include <iostream>
const float PADDING = 8.0f;
const sf::Color highlightColor = sf::Color::Yellow;
void Button::check_mouse_hover(sf::Vector2f mousePos) {
	if (is_hovering(mousePos)) {
		//if (!hovering) std::cout << "mouse over button" << std::endl;
		sprite.setColor(highlightColor);
		hovering = true;
	}
	else {
		hovering = false;
		sprite.setColor(sf::Color::White);
	}
}

bool Button::is_hovering(sf::Vector2f mousePos) {
	return mousePos.x >= pos.x - bounds.x / 2 - PADDING
		&& mousePos.x <= pos.x + bounds.x / 2 + PADDING
		&& mousePos.y >= pos.y - bounds.y / 2 - PADDING
		&& mousePos.y <= pos.y + bounds.y / 2 + PADDING;
}