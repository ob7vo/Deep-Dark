#pragma once
#include "Button.h"
#include <math.h>

const float SLIDER_FRICTION = 0.9f;
const float SLIDER_VELOCITY_BOOST = 1.3f;

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
	void setup(bool lock, std::pair<float, float> bounds){
		lockedX = lock;
		axisBounds = bounds;
		onClick = [this](bool m1) { if (m1) holding = !holding; };
	}
	inline void set_anchored_btns(std::vector<Button*> pBtns) {
		for (auto& pBtn : pBtns) {
			sf::Vector2f offset = pBtn->get_pos() - pos;
			anchoredButtons.emplace_back(pBtn, offset);
		}
	}
	inline void set_anchored_sprites(std::vector<sf::Sprite*> pSprites) {
		for (auto& pSprite : pSprites) {
			sf::Vector2f offset = pSprite->getPosition() - pos;
			anchoredSprites.emplace_back(pSprite, offset);
		}
	}
	inline void set_anchored_sprites(std::vector<sf::Text*> pTexts) {
		for (auto& pText : pTexts) {
			sf::Vector2f offset = pText->getPosition() - pos;
			anchoredTexts.emplace_back(pText, offset);
		}
	}

	inline void update(sf::Vector2i mPos, float dt) {
		if (holding) follow_mouse(mPos);
		else if (has_velocity()) apply_velocity(dt);
	}

	inline void apply_velocity(float dt) {
		sf::Vector2f newPos = pos + (velocity * dt);
		set_pos(newPos);

		velocity *= std::pow(SLIDER_FRICTION, dt * FIXED_FRAMERATE);
		update_anchors();
	}
	inline void follow_mouse(sf::Vector2i mPos) {
		sf::Vector2f oldPos = pos;
		if (lockedX) pos.y = std::clamp((float)mPos.y, axisBounds.first, axisBounds.second);
		else pos.x = std::clamp((float)mPos.x, axisBounds.first, axisBounds.second);

		sprite.setPosition(pos);

		sf::Vector2f instantVelocity = (pos - oldPos) * (float)FIXED_FRAMERATE;
		velocity = (velocity * 0.8f) + (instantVelocity * 0.3f) * SLIDER_VELOCITY_BOOST;

		update_anchors();
	}
	
	inline void update_anchors() const {
		for (auto& [btn, offset] : anchoredButtons) 
			btn->set_pos(pos + offset);
		for (auto& [sprite, offset] : anchoredSprites)
			sprite->setPosition(pos + offset);
		for (auto& [text, offset] : anchoredTexts)
			text->setPosition(pos + offset);
	}
	inline void reset_offsets() {
		for (auto& [pBtn, offset] : anchoredButtons) 
			offset = pBtn->get_pos() - pos;
		for (auto& [sprite, offset] : anchoredSprites)
			offset = sprite->getPosition() - pos;
		for (auto& [text, offset] : anchoredTexts)
			offset = text->getPosition() - pos;
	}
	inline bool has_velocity() const {
		return std::abs(velocity.x) > 0.1f || std::abs(velocity.y) > 0.1f;
	}
	inline bool being_held() const { return holding; }
	inline void set_mouse_hold(bool h) { holding = h; }
};