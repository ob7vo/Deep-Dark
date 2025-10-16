#include "Button.h"
#include <iostream>
const float PADDING = 8.0f;

Button::Button(sf::Vector2f pos, sf::Vector2f bounds, sf::Color color, std::function<void()> onClick = {}) :
	viewportPos(pos), bounds(bounds), baseColor(color), shape(bounds), pos(pos), onClick(onClick){
	shape.setFillColor(baseColor);
	shape.setPosition(pos);
	shape.setOrigin(bounds / 2.f);
}

void Button::check_mouse_hover(sf::Vector2f mousePos) {
	if (is_hovering(mousePos)) {
		if (!hovering) std::cout << "mouse over button" << std::endl;
		shape.setFillColor(sf::Color::Red);
		hovering = true;
	}
	else {
		hovering = false;
		shape.setFillColor(baseColor);
	}
}

bool Button::is_hovering(sf::Vector2f mousePos) {
	return mousePos.x >= pos.x - bounds.x / 2 - PADDING
		&& mousePos.x <= pos.x + bounds.x / 2 + PADDING
		&& mousePos.y >= pos.y - bounds.y / 2 - PADDING
		&& mousePos.y <= pos.y + bounds.y / 2 + PADDING;
}

void Button::draw(sf::RenderWindow& window) {
	window.draw(shape);
}