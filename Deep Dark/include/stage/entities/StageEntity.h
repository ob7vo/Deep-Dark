#pragma once
#include "Animation.h"
#include <json_fwd.hpp>

struct Stage;

struct StageEntity {
	bool readyForRemoval = false;

	bool animating = false;
	Animation anim;
	sf::Sprite sprite = sf::Sprite(defTex);

	int laneInd = 0;
	sf::Vector2f pos;

	StageEntity(sf::Vector2f _pos, int _lane) : laneInd(_lane), pos(_pos) {
		sprite.setPosition(pos);
	}
	virtual ~StageEntity() = default;

	inline int update_animation(float dt) { return anim.update(dt, sprite); }

	virtual void tick(Stage& stage, float deltaTime) {
		int events = anim.update(deltaTime, sprite);

		if (Animation::check_for_event(TRIGGER, events))
			action(stage);
		readyForRemoval = Animation::check_for_event(FINAL_FRAME, events);
	}
	virtual void action(Stage& stage) = 0;
};