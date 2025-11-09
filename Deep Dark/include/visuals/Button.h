#pragma once
#include <SFML\Graphics.hpp>
#include <functional>
#include "Camera.h"
#include <iostream>

class Button
{
protected:
	sf::Vector2f bounds = { 0.f,0.f };
	sf::Vector2f pos = { 0.f,0.f };

	sf::Texture idleTexture = defTexture;
	sf::Texture hoverTexture = defTexture;

public:
	std::function<void(bool)> onClick = NULL;
	sf::Sprite sprite = sf::Sprite(defTexture);
	sf::VertexArray darkOverlay = sf::VertexArray(sf::PrimitiveType::TriangleStrip, 4);

	bool ui = true;
	bool hovering = 0;

	Button() = default;

	bool check_mouse_hover(sf::Vector2i mPos);
	bool is_hovering(sf::Vector2i mPos);
	inline void set_ui_params(sf::Vector2f normPos, sf::Vector2f scale, 
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
	inline void set_world_params(sf::Vector2f pos, sf::Vector2f scale,
		const std::string& path, Camera& cam) {
		cam.set_sprite_params({1.f,1.f}, scale, path, idleTexture, sprite);
		sprite.setPosition(pos);
		this->pos = pos;
		bounds = sprite.getGlobalBounds().size * 1.05f;

		sf::FloatRect _bounds = sprite.getGlobalBounds();
		float left = _bounds.position.x;
		float top = _bounds.position.y;
		float height = _bounds.size.y;
		float width = _bounds.size.x;

		cam.set_dark_overlay(darkOverlay, left, top, width, height);
		ui = false;
	}

	inline sf::Vector2f get_pos() const { return pos; }
	inline void set_pos(sf::Vector2f newPos) { 
		pos = newPos; 
		sprite.setPosition(newPos);
	}
	inline void set_norm_pos(sf::Vector2f norm, Camera& cam) {
		sf::Vector2f newPos = cam.norm_to_pixels(norm);
		set_pos(newPos);
	}
};

