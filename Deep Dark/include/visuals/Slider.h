#pragma once
#include "Button.h"

const float SLIDER_FRICTION = 0.9f;
const float SLIDER_VELOCITY_BOOST = 1.3f;

class sf::Text;

class Slider : public Button {
private:
	sf::Vector2f velocity = { 0.f,0.f };

	bool holding = false;
	std::vector<std::pair<Button*, sf::Vector2f>> anchoredButtons = {};
	std::vector<std::pair<sf::Sprite*, sf::Vector2f>> anchoredSprites = {};
	std::vector<std::pair<sf::Text*, sf::Vector2f>> anchoredTexts = {};

	bool lockedX = false;
	std::pair<float, float> axisBounds; // area where the slider can be dragged
public:
	Slider() = default;
	void setup_slider(bool lock, std::pair<float, float> bounds);

	void set_anchored_btns(std::vector<Button*> pBtns);
	void set_anchored_sprites(std::vector<sf::Sprite*> pSprites);
	void set_anchored_sprites(std::vector<sf::Text*> pTexts);

	void update(sf::Vector2i mPos, float dt);

	void apply_velocity(float dt);
	void follow_mouse(sf::Vector2i mPos);

	void update_anchors() const;
	void reset_offsets();

	inline bool has_velocity() const {
		return std::abs(velocity.x) > 0.1f || std::abs(velocity.y) > 0.1f;
	}
	inline bool being_held() const { return holding; }
	inline void set_mouse_hold(bool h) { holding = h; }
};