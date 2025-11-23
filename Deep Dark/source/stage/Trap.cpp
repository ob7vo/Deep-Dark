#include "Trap.h"
#include "Lane.h"
#include "StageRecord.h"

const float PROC_CHANCE = 100.f;
Trap::Trap(Lane& lane, const nlohmann::json& trap) :  lane(lane){
	int type = trap["trap_type"];
	trapType = static_cast<TrapType>(type);
	triggerRange = trap["trigger_range"];
	attackRange = trap["attack_range"];
	pos = { trap["x_position"], lane.yPos };
	checkTimer = trap["check_timer"];
	dmgValue = trap.value("dmg_value", 0.f);
	if (trap.contains("augment")) {
		AugmentType augType = Augment::string_to_augment_type(trap["augment"]["augment_type"]);
		float statusTime = trap["augment"]["status_time"];
		aug = Augment::status(augType, statusTime);
	}

	animating = false;
	ani = get_trap_animation(trapType);
	ani.reset(sprite);
	sprite.setPosition(pos);
}
bool Trap::in_trigger_range(Unit& unit) const {
	float x = unit.get_pos().x; 
	float y = unit.get_pos().y;
	return x >= triggerRange.first && x <= triggerRange.second &&
	y <= pos.y + TRAP_HEIGHT && y >= pos.y - TRAP_HEIGHT
		&& !(unit.stats->unitTypes & NANO);
}
bool Trap::valid_attack_target(Unit& unit) const {
	return unit.get_pos().x >= attackRange.first && unit.get_pos().x <= attackRange.second &&
		!unit.anim.invincible();
}
bool Trap::enemy_in_trigger_range() const{
	for (auto& unit : lane.enemyUnits)
		if (in_trigger_range(unit))
			return true;
	for (auto& unit : lane.playerUnits)
		if (in_trigger_range(unit))
			return true;
	return false;
}
void Trap::tick( float deltaTime, StageRecord& rec) {
	if (animating) {
		int events = ani.update(deltaTime, sprite);
		if (Animation::check_for_event(AnimationEvent::FINAL_FRAME, events)) {
			timeLeft = checkTimer;
			animating = false;
			ani.reset(sprite);
		}
		if (Animation::check_for_event(AnimationEvent::TRIGGER, events))
			trigger(rec);
	}

	timeLeft -= deltaTime;

	if (timeLeft < 0 && enemy_in_trigger_range()) {
		animating = true;
		timeLeft = 999.f;
		ani.reset(sprite);
	}
}
void Trap::trigger(StageRecord& rec) {
	rec.add_trap_trigger();

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
			unit.movement.push_launch_request(unit);
	for (auto& unit : lane.playerUnits)
		if (valid_attack_target(unit))
			unit.movement.push_launch_request(unit);
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
void Trap::trigger_attack() const {
	std::cout << "trigger_attack" << std::endl;
	attack_lane(lane.enemyUnits);
	attack_lane(lane.playerUnits);
}
void Trap::attack_lane(std::vector<Unit>& units) const {
	for (auto& unit : units) {
		if (!valid_attack_target(unit)) continue;
		if (dmgValue > 0.f) {
			int dmg = trapType == TrapType::PERCENT_DMG ?
				(int)((float)unit.stats->maxHp * dmgValue) : (int)dmgValue;
			if (unit.status.take_damage(unit, dmg))
				unit.causeOfDeath = DeathCause::TRAP;
		}
		if (unit.status.can_proc_status(unit, aug))
			unit.status.add_status_effect(aug);
	}
}

Animation Trap::get_trap_animation(TrapType type) {
	std::vector<std::pair<int, AnimationEvent>> events;
	// starts off as LAUNCH_PAD
	std::string spritePath = "sprites/traps/launch_pad.png";
	sf::Vector2i cellSize = { 96,32 };
	sf::Vector2f origin = { 48, 32 };
	int frames = 16;
	float rate = 0.2f;

	switch (type) {
	case TrapType::LAUNCH_PAD:
		events.emplace_back(8, TRIGGER);
		break;
	case TrapType::TRAP_DOOR: {
		spritePath = "sprites/traps/trap_door.png";
		cellSize.x = 144;
		origin.x = 72;
		frames = 24;
		rate = .3f;
		events.emplace_back(5, TRIGGER);
		events.emplace_back(20, TRIGGER);
		break;
	}
	default: {
		spritePath = "sprites/traps/flat_dmg.png";
		cellSize.x = 32;
		origin.x = 16;
		frames = 25;
		rate = .15f;
		events.emplace_back(13, TRIGGER);
	}
	}

	return Animation(spritePath, frames, rate, cellSize, origin, events, false);
}