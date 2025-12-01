#include "pch.h"
#include "Trap.h"
#include "Stage.h"
#include "StageRecord.h"

const float PROC_CHANCE = 100.f;
Trap::Trap(const nlohmann::json& trap, sf::Vector2f pos, int lane) : StageEntity(pos, lane){
	int type = trap["trap_type"];
	trapType = static_cast<TrapType>(type);
	triggerRange = trap["trigger_range"];
	attackRange = trap["attack_range"];
	checkTimer = trap["check_timer"];
	dmgValue = trap.value("dmg_value", 0.f);

	if (trap.contains("augment")) {
		AugmentType augType = Augment::string_to_augment_type(trap["augment"]["augment_type"]);
		float statusTime = trap["augment"]["status_time"];
		aug = Augment::status(augType, statusTime);
	}

	animating = false;
	anim = get_trap_animation(trapType);
	anim.reset(sprite);
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
bool Trap::enemy_in_trigger_range(Stage& stage) const{
	for (auto& unit : stage.get_lane(laneInd).enemyUnits)
		if (in_trigger_range(unit))
			return true;
	for (auto& unit : stage.get_lane(laneInd).playerUnits)
		if (in_trigger_range(unit))
			return true;
	return false;
}

void Trap::tick(Stage& stage, float deltaTime) {
	if (animating) {
		int events = anim.update(deltaTime, sprite);
		if (Animation::check_for_event(AnimationEvent::FINAL_FRAME, events)) {
			timeLeft = checkTimer;
			animating = false;
			anim.reset(sprite);
		}
		if (Animation::check_for_event(AnimationEvent::TRIGGER, events))
			action(stage);
	}

	timeLeft -= deltaTime;

	if (timeLeft < 0 && enemy_in_trigger_range(stage)) {
		animating = true;
		timeLeft = 999.f;
		anim.reset(sprite);
	}
}
void Trap::action(Stage& stage) {
	switch (trapType) {
	case TrapType::LAUNCH_PAD:
		trigger_launch_pad(stage);
		break;
	case TrapType::TRAP_DOOR:
		trigger_trap_door(stage);
		break;
	default:
		trigger_attack(stage);
		break;
	}
}
void Trap::trigger_launch_pad(Stage& stage) const {
	Lane& lane = stage.get_lane(laneInd);

	for (auto& unit : lane.enemyUnits)
		if (valid_attack_target(unit))
			unit.movement.push_launch_request(unit);
	for (auto& unit : lane.playerUnits)
		if (valid_attack_target(unit))
			unit.movement.push_launch_request(unit);
}
void Trap::trigger_trap_door(Stage& stage) {
	Lane& lane = stage.get_lane(laneInd);

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

void Trap::trigger_attack(Stage& stage) const {
	attack_lane(stage.lanes[laneInd].enemyUnits);
	attack_lane(stage.lanes[laneInd].playerUnits);
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
	std::vector<int> events(25); // 25 is the most amount fo frames that an animatinon here will have

	switch (type) {
	case TrapType::LAUNCH_PAD: {
		std::string spritePath = "sprites/traps/launch_pad.png";
		sf::Vector2i cellSize = { 96,32 };
		sf::Vector2f origin = { 48, 32 };
		int frames = 16;
		float rate = 0.2f;
		events[8] |= TRIGGER;

		return Animation(spritePath, frames, rate, cellSize, origin, events, false);
	}
	case TrapType::TRAP_DOOR: {
		std::string spritePath = "sprites/traps/trap_door.png";
		sf::Vector2i cellSize = { 144, 32 };
		sf::Vector2f origin = { 72, 32 };
		int frames = 24;
		float rate = .3f;
		events[5] |= TRIGGER;
		events[20] |= TRIGGER;

		return Animation(spritePath, frames, rate, cellSize, origin, events, false);
	}
	default: {
		std::string spritePath = "sprites/traps/flat_dmg.png";
		sf::Vector2i cellSize = { 32, 32 };
		sf::Vector2f origin = { 16, 16 };
		int frames = 25;
		float rate = .15f;
		events[13] |= TRIGGER;

		return Animation(spritePath, frames, rate, cellSize, origin, events, false);
	}
	}

	std::cout << "Did not create Trap Animation" << std::endl;
	return Animation();
}