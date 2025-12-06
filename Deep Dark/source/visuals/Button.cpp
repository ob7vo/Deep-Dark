#include "pch.h"
#include "Button.h"
#include "Camera.h"

const float PADDING = 8.0f;
const sf::Color highlightColor = sf::Color::Yellow;

void Button::setup(sf::Vector2f uiPos, sf::Vector2f normScale,
	const Camera& cam, const sf::Texture& texture, sf::IntRect rect) {
	ui = cam.setup_sprite(uiPos, normScale, sprite, texture, rect);
	pos = sprite.getPosition();
	bounds = sprite.getGlobalBounds().size * 1.05f;
}

bool Button::try_mouse_press(sf::Vector2i mPos, bool isM1) const {
	if (!is_hovering(mPos)) return false;
		
	onClick(isM1);
	return true;
}
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

bool Button::is_hovering(sf::Vector2i mPos) const {
	return static_cast<float>(mPos.x) >= pos.x - bounds.x * 0.5f
		&& static_cast<float>(mPos.x) <= pos.x + bounds.x * 0.5f
		&& static_cast<float>(mPos.y) >= pos.y - bounds.y * 0.5f
		&& static_cast<float>(mPos.y) <= pos.y + bounds.y * 0.5f;
}

void Button::set_pos(sf::Vector2f newPos) {
	pos = newPos;
	sprite.setPosition(newPos);
}
void Button::set_norm_pos(sf::Vector2f norm, const Camera& cam) {
	sf::Vector2f newPos = cam.norm_to_pixels(norm);
	set_pos(newPos);
}

void Button::set_texture(const sf::Texture& texture, sf::IntRect rect) {
	if (rect.size.x == 0 || rect.size.y == 0) rect.size = (sf::Vector2i)texture.getSize();

	sprite.setTexture(texture);
	sprite.setTextureRect(rect);
}
