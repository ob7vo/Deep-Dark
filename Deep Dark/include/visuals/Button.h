#pragma once
#include "TextureManager.h"

class Camera; 

class Button
{
protected:
	sf::Vector2f bounds = { 0.f,0.f };
	sf::Vector2f pos = { 0.f,0.f };

public:
	std::function<void(bool)> onClick = nullptr;
	sf::Sprite sprite = sf::Sprite(defTex);

	bool ui = true;
	bool hovering = false;

	Button() = default;

	bool check_mouse_hover(sf::Vector2i mPos);
	bool is_hovering(sf::Vector2i mPos) const;

	void setup(sf::Vector2f uiPos, sf::Vector2f normScale,
		const Camera& cam, const sf::Texture& texture, sf::IntRect rect = {});

	inline sf::Vector2f get_pos() const { return pos; }
	void set_pos(sf::Vector2f newPos);
	void set_norm_pos(sf::Vector2f norm, const Camera& cam);
	void set_texture(const sf::Texture& texture, sf::IntRect rect = {});
};

