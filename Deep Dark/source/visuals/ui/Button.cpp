#include "pch.h"
#include "Button.h"
#include "Camera.h"
#include "Utils.h"

using namespace Math;

const float PADDING = 8.0f;
const sf::Color highlightColor = sf::Color::Yellow;

void Button::setup(sf::Vector2f uiPos, sf::Vector2f normScale, const sf::Texture& texture, sf::IntRect rect) {
	Visual::setupSprite(uiPos, normScale, sprite, texture, rect);
	pos = sprite.getPosition();
	bounds = sprite.getGlobalBounds().size * 1.05f;
}

bool Button::try_mouse_press(sf::Vector2i mPos, bool isM1) const {
	if (!mouseIsHovering(mPos)) return false;
		
	onClick(isM1);
	return true;
}
bool Button::check_mouse_hover(sf::Vector2i mPos) {
	if (!visible) return false;
	bool currentlyHovered = mouseIsHovering(mPos);

	if (!hoveredOver && currentlyHovered) {
		sprite.setColor(highlightColor);
		hoveredOver = true;

		return true;
	}
	// Only reset hovering state if the button was hovering beforehand
	else if (hoveredOver && !currentlyHovered) {
		hoveredOver = false;
		sprite.setColor(sf::Color::White);

		return false;
	}

	return currentlyHovered;
}

bool Button::mouseIsHovering(sf::Vector2i mPos) const {
	return static_cast<float>(mPos.x) >= pos.x - bounds.x * 0.5f
		&& static_cast<float>(mPos.x) <= pos.x + bounds.x * 0.5f
		&& static_cast<float>(mPos.y) >= pos.y - bounds.y * 0.5f
		&& static_cast<float>(mPos.y) <= pos.y + bounds.y * 0.5f;
}

void Button::set_pos(sf::Vector2f newPos) {
	pos = newPos;
	sprite.setPosition(newPos);
}
void Button::set_norm_pos(sf::Vector2f norm) {
	sf::Vector2f newPos = Screen::toPixels(norm);
	set_pos(newPos);
}

void Button::set_texture(const sf::Texture& texture, sf::IntRect rect) {
	if (rect.size.x == 0 || rect.size.y == 0) rect.size = (sf::Vector2i)texture.getSize();

	sprite.setTexture(texture);
	sprite.setTextureRect(rect);
}
