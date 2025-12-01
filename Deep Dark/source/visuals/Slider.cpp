#include "pch.h"
#include "Slider.h"

const int VELOCITY_FRAMERATE = 30;

void Slider::setup_slider(bool lock, std::pair<float, float> bounds) {
	lockedX = lock;
	axisBounds = bounds;
	onClick = [this](bool m1) { if (m1) holding = !holding; };
}
void Slider::set_anchored_btns(std::vector<Button*> pBtns) {
	for (auto& pBtn : pBtns) {
		sf::Vector2f offset = pBtn->get_pos() - pos;
		anchoredButtons.emplace_back(pBtn, offset);
	}
}
void Slider::set_anchored_sprites(std::vector<sf::Sprite*> pSprites) {
	for (auto& pSprite : pSprites) {
		sf::Vector2f offset = pSprite->getPosition() - pos;
		anchoredSprites.emplace_back(pSprite, offset);
	}
}
void Slider::set_anchored_sprites(std::vector<sf::Text*> pTexts) {
	for (auto& pText : pTexts) {
		sf::Vector2f offset = pText->getPosition() - pos;
		anchoredTexts.emplace_back(pText, offset);
	}
}

void Slider::update(sf::Vector2i mPos, float dt) {
	if (holding) follow_mouse(mPos);
	else if (has_velocity()) apply_velocity(dt);
}

void Slider::apply_velocity(float dt) {
	sf::Vector2f newPos = pos + (velocity * dt);
	set_pos(newPos);

	velocity *= std::pow(SLIDER_FRICTION, dt * VELOCITY_FRAMERATE);
	update_anchors();
}
void Slider::follow_mouse(sf::Vector2i mPos) {
	sf::Vector2f oldPos = pos;
	if (lockedX) pos.y = std::clamp((float)mPos.y, axisBounds.first, axisBounds.second);
	else pos.x = std::clamp((float)mPos.x, axisBounds.first, axisBounds.second);

	sprite.setPosition(pos);

	sf::Vector2f instantVelocity = (pos - oldPos) * (float)VELOCITY_FRAMERATE;
	velocity = (velocity * 0.8f) + (instantVelocity * 0.3f) * SLIDER_VELOCITY_BOOST;

	update_anchors();
}

void Slider::update_anchors() const {
	for (auto& [btn, offset] : anchoredButtons)
		btn->set_pos(pos + offset);
	for (auto& [sprite, offset] : anchoredSprites)
		sprite->setPosition(pos + offset);
	for (auto& [text, offset] : anchoredTexts)
		text->setPosition(pos + offset);
}
void Slider::reset_offsets() {
	for (auto& [pBtn, offset] : anchoredButtons)
		offset = pBtn->get_pos() - pos;
	for (auto& [sprite, offset] : anchoredSprites)
		offset = sprite->getPosition() - pos;
	for (auto& [text, offset] : anchoredTexts)
		offset = text->getPosition() - pos;
}