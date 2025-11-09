#pragma once
#include <random>
#include <iostream>
#include "Augment.h"


enum class RequestType {
	NOT_DONE,
	NONE,
	KNOCKBACK,
	TELEPORT,
	FALL,
	SQUASH,
	LAUNCH,
	JUMP,
	LEAP,
	PHASE
};
enum class UnitAnimationState {
	WAITING = -2,
	PHASE = -1,
	MOVING = 0,
	ATTACKING = 1,
	IDLING = 2,
	KNOCKEDBACK = 3,
	FALLING = 4,
	JUMPING = 5,
	IS_PHASING = 6,
	DYING
};
enum class DeathCause {
	NONE = 0,
	UNIT,
	SURGE,
	FALLING,
	TRAP,
	CANNON,
};
const enum UnitType { // comments depict the Types common builds
	NULL_TYPE = 0,
	TYPELESS = 1 << 0, // any architype
	STEEL = 1 << 1, // High damage per hit and good health, low range, mid knockbacks (reds)
	NANO = 1 << 2, // Fast, high dps, high knockback count. Don't TRIGGER traps (can still be hurt by them)
	RUSTED = 1 << 3, // Very Slow and VERY bulky, will idle on gaps
	FLOATING = 1 << 4, // Agile, generally mid-ranged, ignores gaps 
	REANIMEN = 1 << 5, // Fast, low range, high dps, low health, can revive.
	ANCIENT = 1 << 6, // Ignore teleporters, usaully apply short-circuit, good bulk and damage.
	HOLOGRAM = 1 << 7, // Can phase through units, good at applying statuses
	VOIDED = 1 << 8, // Aku-sheilds and have unique voided ability.
	ALL = 1 << 9 // Every Unit has type ALL, easy way to give target to all types
};
enum class SpawnCategory {
	NORMAL,
	BOSS,
	SUMMON
};