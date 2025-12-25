#pragma once
#include "EnumFunctions.h"

// Unit Abilities. A (*) means they only work vs targeted types
enum class AugmentType : size_t {
	NONE = 0,
	SLOW = 1 << 0, // speed is dropped to 0.1 (*)
	OVERLOAD = 1 << 1, // speed and all animations are at half speed (*)
	WEAKEN = 1 << 2, // halves damage (*)
	BLIND = 1 << 3, // halves sight range (*)
	CORRODE = 1 << 4, // doubles damage taken (*)
	SHORT_CIRCUIT = 1 << 5, // nullifies targeter-augments, like Curse in Battle Cats (*)
	VIRUS = 1 << 6, // nullfies immunities (*)
	STRENGTHEN = 1 << 7, // x% more damage when under y% health (*)
	BREAKER = 1 << 8, // Deal 3x damage (*)
	RESIST = 1 << 9, // take 1/4 damage (*)
	SUPERIOR = 1 << 10, // 2x damage and 1/2 damage taken deal (*)
	PLUNDER = 1 << 11, // Enemies drop twice the parts on death when dealing the final blow
	VOID = 1 << 12, // cut 30% of current hp (*)
	SQUASH = 1 << 13, // knockbacked enemies will be sent DOWN a lane (can go through gaps)
	LAUNCH = 1 << 14, // knockbacked enemies will be sent UP a lane (can go through gaps)
	// x% chance to spawns y lasers after attacking attack enemy, hits all lanes, same damage as original attack. 
	// Its Level is the number (y) of strikes
	ORBITAL_STRIKE = 1 << 15,
	// x% chance after every attack to spawn a wave that moves foward for a distance, same damage as original attack
	// Higher Levels lets the wave travel farther
	SHOCK_WAVE = 1 << 16,
	// x% chance after every attack to spawn a stationary wall of fire, same damage as original attack but can hit multiple times
	// Its Level is the max # of hits + how long it lasts
	FIRE_WALL = 1 << 17, 
	SURGE_BLOCKER = 1 << 18, // Grants immunity to and destroys any surge that touches it
	DEATH_SURGE = 1 << 19, // x% to spawn a surge on death, Augment's percentage determines type
	COUNTER_SURGE = 1 << 20, // When hit by a surge, an equal surge is created
	SHOVE = 1 << 21, // small knockback (*)
	SHIELD = 1 << 22, // same as Aku shields
	SHIELD_PIERCE = 1 << 23, // x% chance to instantly break a sheild
	CRITICAL = 1 << 24, // x% to deal dpuble damage
	SURVIVOR = 1 << 25, // x% chance to survive fatal hit on 1 hp
	// The first time it sees an enemy it will become intangible and invisbile to all hostile-sorces
	// as it walks a set distance, but it also cannot interact with them.
	PHASE = 1 << 26,
	CLONE = 1 << 27, // When it dies, it will spawn a clone of itself at x% health
	CODE_BREAKER = 1 << 28, // stops an enemy from cloning if dealt the finishing blow
	LEAP = 1 << 29, // leaps distance when approaching a game, can do this y times
	JUMP = 1 << 30, // jumps to a higher ledge at an open gap, can do this x times
	DROP_BOX = 1ULL << 31, // spawn at x% lane completion
	WARP = 1ULL << 32, // x% chande to teleport struck enemies (*)
	TERMINATE = 1ULL << 33, // Hit enemys with x% health remaining will die 
	LIGHTWEIGHT = 1ULL << 34, // Gets knockedback 1.5x farther
	HEAVYWEIGHT = 1ULL << 35, // Gets knockedback .7x farther (or SHORTER i guess, can't be launched
	BULLY = 1ULL << 36, // Knocksback enemies 1.5x farther (*)
	SALVAGE = 1ULL << 37, // Killing an enemy will summon a Unit
	TRANSFORM = 1ULL << 38, // When an enemy(boss) dies, they transoform into their next gear.
	SELF_DESTRUCT = 1ULL << 39, // Units releases a damaging explosion after death
	FRAGILE = 1ULL << 40, // Unit dies after attacking
	PROJECTILE = 1ULL << 41, // Launches a projectile when attacking,
	DEFLECT = 1ULL << 42, // x% chance to deflect projectiles (That are deflectable)
	ROUGH = 1ULL << 43 // Projectiles that hit the unit will take extra chip dmg
};

constexpr AugmentType operator|(AugmentType a, AugmentType b) {
	return static_cast<AugmentType>(
		static_cast<std::size_t>(a) |
		static_cast<std::size_t>(b)
		);
}
constexpr AugmentType operator&(AugmentType a, AugmentType b) {
	return static_cast<AugmentType>(
		static_cast<std::size_t>(a) &
		static_cast<std::size_t>(b)
		);
}
constexpr AugmentType operator~(AugmentType a) {
	using U = std::underlying_type_t<AugmentType>;
	return static_cast<AugmentType>(~static_cast<U>(a));
}

constexpr AugmentType& operator|=(AugmentType& a, AugmentType b) {
	a = a | b;
	return a;
}