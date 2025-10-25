#pragma once
#include <SFML\Graphics.hpp>
#include <functional>
#include "Camera.h"
#include <iostream>

class Button
{
	sf::Vector2f bounds = { 0.f,0.f };
	sf::Vector2f pos = { 0.f,0.f };

	sf::Texture idleTexture = defTexture;
	sf::Texture hoverTexture = defTexture;

public:
	std::function<void()> onClick = NULL;
	sf::Sprite sprite = sf::Sprite(defTexture);
	sf::VertexArray darkOverlay = sf::VertexArray(sf::PrimitiveType::TriangleStrip, 4);
	bool hovering = 0;

	Button() = default;

	bool check_mouse_hover(sf::Vector2i mPos);
	bool is_hovering(sf::Vector2i mPos);
	inline void set_new_params(sf::Vector2f normPos, sf::Vector2f scale, 
		const std::string& path, Camera& cam) {
		cam.set_sprite_params(normPos, scale, path, idleTexture, sprite);
		pos = sprite.getPosition();
		bounds = sprite.getGlobalBounds().size * 1.05f;

		sf::FloatRect _bounds = sprite.getGlobalBounds();
		float left = _bounds.position.x;
		float top = _bounds.position.y;
		float height = _bounds.size.y;
		float width = _bounds.size.x;

		cam.set_dark_overlay(darkOverlay, left, top, width, height);
	}
};

