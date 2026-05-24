#pragma once
#include "EnumFunctions.h"

// Unit Abilities. A (*) means they only work vs targeted types
enum class AugmentType : size_t {
	NONE = 0,
	SLOW = 1 << 0, // Movespeed is dropped to 0.1 (*)
	OVERLOAD = 1 << 1, // Movespeed and all animations are at half speed (*)
	WEAKEN = 1 << 2, // halves damage (*)
	BLIND = 1 << 3, // halves sight range (*)
	BOLT = 1 << 4, // affected Units cannot be knocked-back in any way (*)
	CORRODE = 1 << 5, // increases damage taken by x% (*)
	SHORT_CIRCUIT = 1 << 6, // nullifies target-augments, like Curse in Battle Cats (*)
	VIRUS = 1 << 7, // nullifies immunities (*)
	OVERCLOCK = 1 << 8, // Movespeed and animations become x% faster after surviving y seconds
	REINFORCE = 1 << 9, // Decreases damage taken by x% 
	SCOPE = 1 << 10, // Increases sight and attack range by 50%
	STRENGTHEN = 1 << 11, // x% more damage when under y% health (*)
	BREAKER = 1 << 12, // Deal 3x damage (*)
	RESIST = 1 << 13, // take 1/4 damage (*)
	SUPERIOR = 1 << 14, // 2x damage and 1/2 damage taken deal (*)
	PLUNDER = 1 << 15, // Enemies drop twice the parts on death when dealing the final blow
	VOID = 1 << 16, // cut 30% of current hp (*)
	SQUASH = 1 << 17, // knockbacked enemies will be sent DOWN a lane (can go through gaps)
	LAUNCH = 1 << 18, // knockbacked enemies will be sent UP a lane (can go through gaps)
	// x% chance to spawns y lasers after attacking attack enemy, hits all lanes, same damage as original attack. 
	// Its Level is the number (y) of strikes
	ORBITAL_STRIKE = 1 << 19,
	// x% chance after every attack to spawn a wave that moves forward for a distance, same damage as original attack
	// Higher Levels lets the wave travel farther
	SHOCK_WAVE = 1 << 20,
	// x% chance after every attack to spawn a stationary wall of fire, same damage as original attack but can hit multiple times
	// Its Level is the max # of hits + how long it lasts
	FIRE_WALL = 1 << 21, 
	SURGE_BLOCKER = 1 << 22, // Grants immunity to and destroys any surge that touches it
	DEATH_SURGE = 1 << 23, // x% to spawn a surge on death, Augment's percentage determines type
	COUNTER_SURGE = 1 << 24, // When hit by a surge, an equal surge is created
	SHOVE = 1 << 25, // small knockback (*)
	SHIELD = 1 << 26, // same as Aku shields
	SHIELD_PIERCE = 1 << 27, // x% chance to instantly break a shield
	CRITICAL = 1 << 28, // x% to deal double damage
	SURVIVOR = 1 << 29, // x% chance to survive fatal hit on 1 hp
	// The first time it sees an enemy it will become intangible and invisibile to all hostile-sorces
	// as it walks a set distance, but it also cannot interact with them.
	PHASE = 1 << 30,
	CLONE = 1ULL << 31, // When it dies, it will spawn a clone of itself at x% health
	CODE_BREAKER = 1ULL << 32, // stops an enemy from cloning if dealt the finishing blow
	LEAP = 1ULL << 33, // leaps distance when approaching a game, can do this y times
	JUMP = 1ULL << 34, // jumps to a higher ledge at an open gap, can do this x times
	DROP_BOX = 1ULL << 35, // spawn at x% lane completion
	WARP = 1ULL << 36, // x% chance to teleport struck enemies (*)
	TERMINATE = 1ULL << 37, // Hit enemies with x% health remaining will die 
	LIGHTWEIGHT = 1ULL << 38, // Received knockback force is 50% greater
	HEAVYWEIGHT = 1ULL << 39, // Received knockback force is 30% less
	BULLY = 1ULL << 40, // Unit will Knockback enemies 50% father (*)
	SALVAGE = 1ULL << 41, // Killing an enemy will summon a Unit
	TRANSFORM = 1ULL << 42, // When an enemy(boss) dies, they transform into their next gear.
	SELF_DESTRUCT = 1ULL << 43, // Units releases a damaging explosion after death
	FRAGILE = 1ULL << 44, // Unit dies after attacking
	PROJECTILE = 1ULL << 45, // Launches a projectile when attacking,
	DEFLECT = 1ULL << 46, // x% chance to deflect projectiles (That are deflectable)
	ROUGH = 1ULL << 47, // Projectiles that hit the unit will take extra chip dmg
	LINK = 1ULL << 48, // When hit with a status, negative statuses wil be applied to enemies and positive statuses will be applied to allies.
	SYPHON = 1ULL << 49, // Heals up one knockback threshold after killing x enemies
	DISCHARGE = 1ULL << 50 // Every x seconds, the damage taken over that time period is released as an explosion. (Exclusive to Capcitor types)
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