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

	StageEntity(sf::Vector2f _pos, int _lane);
	virtual ~StageEntity();

	virtual int update_animation(Stage& stage, float dt);
	virtual void tick(Stage& stage, float deltaTime);
	virtual void action(Stage& stage);
};