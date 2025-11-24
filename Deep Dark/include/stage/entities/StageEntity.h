#pragma once
#include "Animation.h"

struct Stage;

struct StageEntity {
	bool readyForRemoval = false;

	bool animating = false;
	Animation ani;
	sf::Sprite sprite = sf::Sprite(defTex);

	int laneInd = 0;
	sf::Vector2f pos;

	StageEntity(sf::Vector2f _pos, int _lane) : pos(_pos), laneInd(_lane) {}
	virtual ~StageEntity() = default;

	inline int update_animation(float dt) { return ani.update(dt, sprite); }

	virtual void tick(Stage& stage, float deltaTime) {
		int events = ani.update(deltaTime, sprite);

		if (Animation::check_for_event(TRIGGER, events))
			action(stage);
		readyForRemoval = Animation::check_for_event(FINAL_FRAME, events);
	}
	virtual void action(Stage& stage) = 0;
};