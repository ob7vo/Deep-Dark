#pragma once
#include "SFML\Graphics\Rect.hpp"
struct Bounds {
	sf::FloatRect rect = {};
	float left = 0;
	float right = 0;
	float top = 0;
	float bottom = 0;

	Bounds() = default;
	Bounds(sf::FloatRect rect) { set_bounds(rect); }
	inline void set_bounds(sf::FloatRect rect) {
		this->rect = rect;
		left = rect.position.x;
		top = rect.position.y;
		right = left + rect.size.x;
		bottom = top + rect.size.y;
	}
};