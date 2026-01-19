#pragma once
#include "TextureManager.h"
#include <SFML/Graphics/Sprite.hpp>

const float DOUBLE_CLICK_WINDOW = 0.3f;

struct MouseCursor {
public:
	sf::Vector2f worldPos = { 0.f, 0.f };
	sf::Vector2i screenPos = { 0,0 };

	sf::Sprite ui = sf::Sprite(defTex);
	sf::Texture UITexture;

	sf::Vector2f dragOrigin{ 0.f,0.f };
	sf::Vector2f velocity{ 0.f,0.f };
	bool dragging = false;

	float lastClickTime = 0.f;

	MouseCursor() = default;

	inline void set_pos(sf::Vector2f mPos, sf::Vector2i mS_Pos) {
		worldPos = mPos; screenPos = mS_Pos;
		ui.setPosition(static_cast<sf::Vector2f>(screenPos));
	}
	inline bool has_velocity() const {
		return std::abs(velocity.x) > 0.1f || std::abs(velocity.y) > 0.1f;
	}
};

