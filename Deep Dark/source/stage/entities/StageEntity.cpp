#include "pch.h"
#include "StageEntity.h"

StageEntity::StageEntity(sf::Vector2f _pos, int _lane) : laneInd(_lane), pos(_pos) {
	sprite.setPosition(pos);
}
StageEntity::~StageEntity() = default;

int StageEntity::update_animation(Stage& stage, float dt) {
	int events = anim.update(dt, sprite);

	if (Animation::check_for_event(TRIGGER, events))
		action(stage);
	readyForRemoval = Animation::check_for_event(FINAL_FRAME, events);

	return events;
}

void StageEntity::tick(Stage& stage, float deltaTime) {
	update_animation(stage, deltaTime);
}
// Base doesnt have an action that uses Stage, so this isn't a pure virtual
void StageEntity::action(Stage& stage) { return; }