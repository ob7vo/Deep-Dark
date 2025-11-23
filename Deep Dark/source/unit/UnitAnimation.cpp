#include "UnitAnimation.h"
#include "Unit.h"

UnitAnimation::UnitAnimation(UnitAniMap* map) : marker({ 13.f,35.f }), 
aniMap(map){
	marker.setFillColor(sf::Color::Cyan);
	marker.setOrigin(marker.getSize());

	start(UnitAnimationState::MOVE);
}
void UnitAnimation::start(UnitAnimationState newState) {
	state = newState;
	(*aniMap)[newState].reset(time, currentFrame, sprite);
}
void UnitAnimation::start_idle_or_attack_animation(Unit& unit) {
	if (unit.combat.cooldown <= 0) {
		unit.combat.hitIndex = 0;
		start(UnitAnimationState::ATTACK);
	}
	else start(UnitAnimationState::IDLE);
}
int UnitAnimation::update(float deltaTime) {
	int events = (*aniMap)[state].update(time, currentFrame, deltaTime, sprite);
	return events;
}
void UnitAnimation::set_position(sf::Vector2f pos) {
	sprite.setPosition(pos);
	marker.setPosition(pos);
}
