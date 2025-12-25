#include "pch.h"
#include "UnitAnimation.h"
#include "Unit.h"

constexpr sf::Color HURTBOX_COLOR = { 0, 255, 255, static_cast<uint8_t>(255 * 0.2f) };
UnitAnimation::UnitAnimation(UnitAniMap* map, const UnitStats* stats) : aniMap(map) {
	hurtbox.setSize(stats->hurtBox);
	hurtbox.setFillColor(HURTBOX_COLOR);

	sf::Vector2f origin = hurtbox.getSize();
	if (!stats->is_player()) origin.x = 0.f;
	hurtbox.setOrigin(origin);

	start(UnitAnimationState::MOVE);

	// These are for the debuging/creation process to make configing
	// Unit sprites easier. Will be removed in final build
	if (UnitData::shouldFlipSprite(stats->id)) sprite.setScale({ -1, 1 });
	sprite.setColor(UnitData::getGearColor(stats->id, stats->gear));
}

void UnitAnimation::draw(sf::RenderWindow& window) const {
	window.draw(sprite);
	window.draw(hurtbox);
}
void UnitAnimation::start(UnitAnimationState newState) {
	std::cout << "staritng Unit animation" << std::endl;
	state = newState;
	player.start(&(*aniMap)[newState], sprite);

	update_visual_state();
}

void UnitAnimation::update_visual_state() {
	sf::Color newColor = sprite.getColor();

	switch (state) {
	case UnitAnimationState::PHASE_ACTIVE:
		// I decided I actually want phasing units to be slightly visible
		newColor.a = static_cast<uint8_t>(255 * 0.2f);
		break;
	default:
		newColor.a = 255;
		break;
	}

	sprite.setColor(newColor);
}
void UnitAnimation::enter_is_phasing_state() {
	state = UnitAnimationState::PHASE_ACTIVE;

	update_visual_state();
	player.start(&(*aniMap)[UnitAnimationState::MOVE], sprite);
}

void UnitAnimation::start_idle_or_attack_animation(Unit& unit) {
	if (unit.combat.cooldown <= 0) {
		unit.combat.hitIndex = 0;
		start(UnitAnimationState::ATTACK);
	}
	else start(UnitAnimationState::IDLE);
}
void UnitAnimation::start_move_idle_or_attack(Unit& unit) {
	if (unit.enemy_is_in_sight_range())
		start_idle_or_attack_animation(unit);
	else start(UnitAnimationState::MOVE);
}

void UnitAnimation::set_position(sf::Vector2f pos) {
	sprite.setPosition(pos);
	hurtbox.setPosition(pos);
}
