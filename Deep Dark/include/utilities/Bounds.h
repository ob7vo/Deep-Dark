#pragma once
#include <SFML/Graphics/Rect.hpp>

struct Bounds {
	sf::FloatRect rect = {};
	float left = 0;
	float right = 0;
	float top = 0;
	float bottom = 0;

	Bounds() = default;
	explicit Bounds(sf::FloatRect rect) { set_bounds(rect); }

	inline void set_bounds(sf::FloatRect newRect) {
		rect = newRect;
		left = newRect.position.x;
		top = newRect.position.y;
		right = left + newRect.size.x;
		bottom = top + newRect.size.y;
	}
};