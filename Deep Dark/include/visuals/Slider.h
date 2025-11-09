#pragma once
#include "Button.h"
#include <math.h>

const float SLIDER_FRICTION = 0.9f;
const float SLIDER_VELOCITY_BOOST = 1.3f;

class Slider : public Button {
	sf::Vector2f velocity = { 0.f,0.f };

	bool holding = false;
	std::vector<std::pair<Button*, sf::Vector2f>> anchoredButtons = {};

	bool lockedX = false;
	std::pair<float, float> axisBounds; // area where the slider can be dragged
public:
	Slider() = default;
	void setup(bool lock, std::pair<float, float> bounds,
		std::vector<Button*> pBtns){
		lockedX = lock;
		axisBounds = bounds;
		onClick = [this](bool m1) { if (m1) holding = !holding; };
		
		for (auto& pBtn : pBtns) {
			sf::Vector2f offset = pBtn->get_pos() - pos;
			anchoredButtons.emplace_back(pBtn, offset);
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
		update_btns();
	}
	inline void follow_mouse(sf::Vector2i mPos) {
		sf::Vector2f oldPos = pos;
		if (lockedX) pos.y = std::clamp((float)mPos.y, axisBounds.first, axisBounds.second);
		else pos.x = std::clamp((float)mPos.x, axisBounds.first, axisBounds.second);

		sprite.setPosition(pos);

		sf::Vector2f instantVelocity = (pos - oldPos) * (float)FIXED_FRAMERATE;
		velocity = (velocity * 0.8f) + (instantVelocity * 0.3f) * SLIDER_VELOCITY_BOOST;

		update_btns();
	}
	
	inline void update_btns() {
		for (auto& [btn, offset] : anchoredButtons) 
			btn->set_pos(pos + offset);
	}
	inline void reset_offsets() {
		for (auto& [pBtn, offset] : anchoredButtons) 
			offset = pBtn->get_pos() - pos;
	}
	inline bool has_velocity() {
		return std::abs(velocity.x) > 0.1f || std::abs(velocity.y) > 0.1f;
	}
	inline bool being_held() { return holding; }
	inline void set_mouse_hold(bool h) { holding = h; }
};