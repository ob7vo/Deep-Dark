#include "Trap.h"
#include "Lane.h"
const float PROC_CHANCE = 100.f;
Trap::Trap(Lane& lane, const nlohmann::json& trap) : sprite(Animation::def_texture()), 
lane(lane){
	int type = trap["trap_type"];
	trapType = static_cast<TrapType>(type);
	triggerRange = trap["trigger_range"];
	attackRange = trap["attack_range"];
	pos = { trap["x_position"], lane.yPos };
	checkTimer = trap["check_timer"];
	dmgValue = trap.value("dmg_value", 0.f);
	if (trap.contains("augment")) {
		AugmentType augType = UnitStats::convert_string_to_augment(trap["augment"]["augment_type"]);
		float statusTime = trap["augment"]["status_time"];
		aug = Augment::status(augType, statusTime);
	}

	animating = false;
	ani = get_trap_animation(trapType);
	sprite.setTexture(ani.texture);
	ani.reset(sprite);
	sprite.setPosition(pos);

}
bool Trap::in_trigger_range(Unit& unit) {
	return unit.pos.x >= triggerRange.first && unit.pos.x <= triggerRange.second;
	unit.pos.y <= pos.y + TRAP_HEIGHT && unit.pos.y >= pos.y - TRAP_HEIGHT
		&& !unit.nano_type();
}
bool Trap::valid_attack_target(Unit& unit) {
	return unit.pos.x >= attackRange.first && unit.pos.x <= attackRange.second &&
		!unit.invincible() && !unit.pending_death();
}
bool Trap::enemy_in_trigger_range() {
	for (auto& unit : lane.enemyUnits)
		if (in_trigger_range(unit))
			return true;
	for (auto& unit : lane.playerUnits)
		if (in_trigger_range(unit))
			return true;
	return false;
}
void Trap::tick(sf::RenderWindow& window, float deltaTime) {
	if (animating) {
		int events = ani.update(deltaTime, sprite);
		if (Animation::check_for_event(AnimationEvent::FINAL_FRAME, events)) {
			timeLeft = checkTimer;
			animating = false;
			ani.reset(sprite);
		}
		if (Animation::check_for_event(AnimationEvent::TRIGGER, events)) {
			trigger();
		}
	}

	window.draw(sprite);
	timeLeft -= deltaTime;

	if (timeLeft < 0 && enemy_in_trigger_range()) {
		animating = true;
		timeLeft = 999.f;
		ani.reset(sprite);
	}
}
void Trap::trigger() {
	switch (trapType) {
	case TrapType::LAUNCH_PAD:
		trigger_launch_pad();
		break;
	case TrapType::TRAP_DOOR:
		trigger_trap_door();
		break;
	default:
		trigger_attack();
		break;
	}
}
void Trap::trigger_launch_pad() {
	for (auto& unit : lane.enemyUnits)
		if (valid_attack_target(unit))
			unit.push_launch_request();
	for (auto& unit : lane.playerUnits)
		if (valid_attack_target(unit))
			unit.push_launch_request();
}
void Trap::trigger_trap_door() {
	if (!triggered) {
		lane.gaps.emplace_back(attackRange);
		lane.add_shape(attackRange);
	}
	else if (!lane.gaps.empty()) {
		lane.gaps.pop_back();
		lane.gapsShapes.pop_back();
	}
	triggered = !triggered;
}
void Trap::trigger_attack() {
	for (auto& unit : lane.enemyUnits)
		if (valid_attack_target(unit)) {
			if (dmgValue > 0.f) {
				int dmg = trapType == TrapType::PERCENT_DMG ?
					(int)(unit.stats->maxHp * dmgValue) : (int)dmgValue;
				if (unit.take_damage(dmg))
					unit.causeOfDeath = DeathCause::TRAP;
			}
			if (unit.can_proc_status(aug))	
				unit.add_status_effect(aug);
		}
	for (auto& unit : lane.playerUnits)
		if (valid_attack_target(unit)) {
			if (dmgValue > 0.f) {
				int dmg = trapType == TrapType::PERCENT_DMG ?
					(int)(unit.stats->maxHp * dmgValue) : (int)dmgValue;
				if (unit.take_damage(dmg))
					unit.causeOfDeath = DeathCause::TRAP;
			}
			if (unit.can_proc_status(aug))	
				unit.add_status_effect(aug);
		}
}

Animation Trap::get_trap_animation(TrapType type) {
	std::vector<std::pair<int, AnimationEvent>> events;
	// starts off as LAUNCH_PAD
	std::string spritePath = "sprites/traps/launch_pad.png";
	int textureSize[2] = { 1536, 32 }, cellSize[2] = { 96,32 };
	int frames = 16;
	float rate = 0.2f;

	switch (type) {
	case TrapType::LAUNCH_PAD:
		events.emplace_back(8, TRIGGER);
		break;
	case TrapType::TRAP_DOOR: {
		spritePath = "sprites/traps/trap_door.png";
		textureSize[0] = 3456;
		cellSize[0] = 144;
		frames = 24;
		rate = .3f;
		events.emplace_back(5, TRIGGER);
		events.emplace_back(20, TRIGGER);
		break;
	}
	default: {
		spritePath = "sprites/traps/flat_dmg.png";
		textureSize[0] = 800;
		cellSize[0] = 32;
		frames = 25;
		rate = .15f;
		events.emplace_back(13, TRIGGER);
	}
	}
	return Animation(spritePath, frames, rate, textureSize, cellSize, events, false);
}