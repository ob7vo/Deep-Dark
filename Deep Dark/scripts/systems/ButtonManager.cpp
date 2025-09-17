#include "ButtonManager.h"
ButtonManager::ButtonManager() {
	buttons.reserve(30);
}
void ButtonManager::check_mouse_hover(sf::Vector2f mousePos) {
	for (auto& button : buttons) 
		button.check_mouse_hover(mousePos);
}
void ButtonManager::register_click(sf::Vector2f mousePos) {
	for (auto& button : buttons) 
		if (button.is_hovering(mousePos)) 
			if (button.onClick) {
				button.onClick();
				return;
			}
}

void ButtonManager::draw(sf::RenderWindow& window) {
	for (auto& button : buttons)
		button.draw(window);
}
void ButtonManager::tick(sf::RenderWindow& window, sf::Vector2f mousePos) {
	draw(window);
	check_mouse_hover(mousePos);
}
