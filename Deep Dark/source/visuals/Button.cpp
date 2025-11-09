#include "Button.h"
#include <iostream>
const float PADDING = 8.0f;
const sf::Color highlightColor = sf::Color::Yellow;
bool Button::check_mouse_hover(sf::Vector2i mPos) {
	if (is_hovering(mPos)) {
		//if (!hovering) std::cout << "mouse over button" << std::endl;
		sprite.setColor(highlightColor);
		hovering = true;
		return true;
	}
	else {
		hovering = false;
		sprite.setColor(sf::Color::White);
		return false;
	}
}

bool Button::is_hovering(sf::Vector2i mPos) {
	return mPos.x >= pos.x - bounds.x * 0.5f
		&& mPos.x <= pos.x + bounds.x * 0.5f
		&& mPos.y >= pos.y - bounds.y * 0.5f
		&& mPos.y <= pos.y + bounds.y * 0.5f;
}