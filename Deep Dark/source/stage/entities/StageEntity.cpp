#include "pch.h"
#include "StageEntity.h"
#include "Unit.h"
#include "Utils.h"

StageEntity::StageEntity(sf::Vector2f _pos, int _lane) : 
	sprite(defTex),
	laneInd(_lane),
	pos(_pos)
{
	sprite.setPosition(pos);
}
StageEntity::~StageEntity() = default;

AnimationEvent StageEntity::update_animation(Stage& stage, float dt) {
	AnimationEvent events = animPlayer.update(dt, sprite);

	if (any(events & AnimationEvent::TRIGGER))
		action(stage);
	if (any(events & AnimationEvent::FINAL_FRAME))
		readyForRemoval = true;

	return events;
}

void StageEntity::tick(Stage& stage, float deltaTime) {
	update_animation(stage, deltaTime);
}
// Base doesnt have an action that uses Stage, so this isn't a pure virtual
void StageEntity::action(Stage& stage) { return; }
bool StageEntity::collides(sf::FloatRect entityBounds, const Unit& unit) const {
	return Collision::AABB(entityBounds, unit.getBounds());
}
