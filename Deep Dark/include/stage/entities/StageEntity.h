#pragma once
#include "Animation.h"

struct Stage;
class Unit;

struct StageEntity {
	bool readyForRemoval = false;

	bool animating = false;
	AnimationPlayer animPlayer;
	sf::Sprite sprite;

	int laneInd = 0;
	sf::Vector2f pos;

	StageEntity(sf::Vector2f _pos, int _lane);
	virtual ~StageEntity();

	virtual AnimationEvent update_animation(Stage& stage, float dt);
	virtual void tick(Stage& stage, float deltaTime);
	virtual void action(Stage& stage);

	bool collides(sf::FloatRect bounds, const Unit& unit) const;
};