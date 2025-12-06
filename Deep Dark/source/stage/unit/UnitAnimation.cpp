#include "pch.h"
#include "UnitAnimation.h"
#include "Unit.h"

constexpr sf::Color HURTBOX_COLOR = { 0, 255, 255, 75 };
UnitAnimation::UnitAnimation(UnitAniMap* map, const UnitStats* stats) : aniMap(map){
	hurtbox.setSize(stats->hurtBox);
	hurtbox.setFillColor(HURTBOX_COLOR);

	sf::Vector2f origin = hurtbox.getSize();
	if (!stats->is_player()) origin.x = 0.f;
	hurtbox.setOrigin(origin);

	start(UnitAnimationState::MOVE);
}

void UnitAnimation::draw(sf::RenderWindow& window) const {
	window.draw(sprite);
	window.draw(hurtbox);
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
	hurtbox.setPosition(pos);
}
