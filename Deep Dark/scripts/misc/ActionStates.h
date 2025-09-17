#pragma once
#include <random>

enum class RequestType {
	TELEPORT,
	FALL,
	SQUASH,
	LAUNCH,
	JUMP
};
enum class UnitAnimationState {
	PHASE = -1,
	MOVING = 0,
	ATTACKING = 1,
	IDILING = 2,
	KNOCKEDBACK = 3,
	FALLING = 4,
	JUMPING = 5,
	IS_PHASING = 6,
	DYING
};
const enum AugmentType {
	NONE = 0,
	SLOW = 1 << 0, // speed is dropped to 0.1 (traits)
	OVERLOAD = 1 << 1, // speed and all animations are at half speed (deltaTime / 2) (traits)
	WEAKEN = 1 << 2, // lowers damage (traits)
	BLIND = 1 << 3, // halves sight range (traits)
	SHORT_CIRCUIT = 1 << 4, // nullifies trait-augments (curse)
	VIRUS = 1 << 5, // nullfies immunities (traits)
	BREAKER = 1 << 6, // Deal 3x damage (traits)
	RESIST = 1 << 7, // take 1/4 damage (traits)
	SUPERIOR = 1 << 8, // deal x% more damage when under y% health
	STRENGTHEN = 1 << 9, // 2x damage and 1/2 damage taken (traits)
	PLUNDER = 1 << 10, // Enemy units drop twice the parts on death
	VOID = 1 << 11, // cut 30% of current hp (traits)
	SQUASH = 1 << 12, // knockbacked enemies will be sent DOWN a lane (through gaps)
	LAUNCH = 1 << 13, // knockbacked enemies will be sent UP a lane (through gaps)
	ORBITAL_STRIKE = 1 << 14, // spawns one laser directly on an attack enemy, hits all lanes, same damage as original attack, and has a width
	SHOCK_WAVE = 1 << 15, // same as battle cats waves, wave blocker = immunity to wave + last bit flipped
	FIRE_WALL = 1 << 16, // same as battle cats surges
	SURGE_BLOCKER = 1 << 17,
	DEATH_SURGE = 1 << 18, // x% to spawn a surge on death, Augment's percentage determines type
	COUNTER_SURGE = 1 << 19,
	SHOVE = 1 << 20, // small knockback (traits)
	SHIELD = 1 << 21, // same as Aku shields
	SHIELD_PIERCE = 1 << 22,
	CRITICAL = 1 << 23, // x% to deal dpuble damage
	SURVIVOR = 1 << 24, // x% chance to survive fatal hit on 1 hp
	PHASE = 1 << 25, // same as zombie burrow
	CLONE = 1 << 26, // same as zombie revive
	CODE_BREAKER = 1 << 27, // stops cloning
	LEAP = 1 << 28, // leaps distance when approaching a game, can do this y times
	JUMP = 1 << 29 // jumps to a higher ledge at an open gap, can do this x times
};
const enum UnitType { // comments depict the Types common builds
	NULL_TYPE = 0,
	TYPELESS = 1 << 0, // any architype
	STEEL = 1 << 1, // High damage per hit and good health, low range, mid knockbacks (reds)
	NANO = 1 << 2, // Fast, high dps, high knockback count (like black trait)
	RUSTED = 1 << 3, // Very Slow and VERY bulky, will idle on gaps
	FLOATING = 1 << 4, // Agile, generally mid-ranged, ignores gaps 
	REANIMEN = 1 << 5, // Fast, low range, high dps, low health, can revive.
	ANCIENT = 1 << 6, // Ignore teleporters, usaully apply short-circuit, good bulk and damage.
	HOLOGRAM = 1 << 7, // Can phase through units, good at applying statuses
	VOIDED = 1 << 8, // Aku-sheilds and have unique voided ability.
	ALL = 1 << 9 // Every Unit has type ALL, easy way to give target to all types
};
struct Augment {
	AugmentType augType = NONE;
	/// <summary> Duration of status effects or distance of surge placement </summary>
	float value = 0.0f;
	float percentage = 1.0f;
	int surgeLevel = 1;

	Augment() : augType(NONE), value(0.0f), percentage(1.0f), surgeLevel(1) {}
	Augment(AugmentType aug, float val, float percentage, int lvl = 1) :
		augType(aug), value(val), percentage(percentage), surgeLevel(lvl) {}


	inline bool is_surge() const {
		return augType & AugmentType::ORBITAL_STRIKE ||
			augType & AugmentType::FIRE_WALL || augType & AugmentType::SHOCK_WAVE;
	}
	inline bool is_status_effect() const { return augType <= 32; }
	inline bool is_damage_modifier() const { return augType >= 64 && augType <= 256; }
};
