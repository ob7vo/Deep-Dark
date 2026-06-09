#pragma once
#include "EnumFunctions.h"

///<summary>Unit Abilities. 
///A (*) means they only work vs targeted types
///A (-) means a Unit only has to ahve teh ability for its effect. I has no actual values.
///A (--- IGNORE ---) means that the Augment's values are still being worked on and may change in the future, so they should not be used in any calculations yet.
///A The augments are ordered to be used in bitmaps
///</summary>
enum class AugmentType : std::size_t { 
	// nuh uh
	NONE = 0,

	// Movespeed is dropped to 0.1 (*)
	SLOW = 1 << 0, 

	// Movespeed and all animations are at half speed (*)
	OVERLOAD = 1 << 1, 

	// halves damage (*)
	WEAKEN = 1 << 2, 

	// halves sight range (*)
	BLIND = 1 << 3,

	// affected Units cannot be knocked-back in any way (*)
	BOLT = 1 << 4, 

	// increases damage taken by x% (*)
	CORRODE = 1 << 5,

	// nullifies target-augments (like Curse in Battle Cats) (*)
	SHORT_CIRCUIT = 1 << 6,

	// All Immunities are ignored while status is active (*)
	VIRUS = 1 << 7, 

	// Dying with this status will cause a team-targetted explosion dealing damage worth x% of their max hp (*)
	DETONATE = 1 << 8,

	// Movespeed and animations become x% faster after surviving y seconds
	OVERCLOCK = 1 << 9, 

	// Decreases damage taken by x% when under y% hp
	REINFORCE = 1 << 10, 

	// Increases sight and attack range by x% after going under y% hp
	SCOPE = 1 << 11, 

	// x% more damage when under y% health (*)
	STRENGTHEN = 1 << 12,

	// Deal x% more damage to target types (*)
	BREAKER = 1 << 13,

	// take x% less damage from target types (*)
	RESIST = 1 << 14, 

	// Enemies drop twice the parts on death when dealing the final blow (-)
	PLUNDER = 1 << 15,

	// x% chance to deal y% of enemies max HP on hit(*)
	VOID = 1 << 16,

	// knockbacked enemies will be sent DOWN a lane (can go through gaps) (-)
	SQUASH = 1 << 17,

	// knockbacked enemies will be sent UP a lane (can go through gaps) (-)
	LAUNCH = 1 << 18, 

	// x% chance to spawns y lasers after attacking attack enemy, hits all lanes, same damage as original attack. 
	// Its Level is the number (y) of strikes
	ORBITAL_STRIKE = 1 << 19,

	// x% chance after every attack to spawn a wave that moves forward for a distance, same damage as original attack
	// Higher Levels lets the wave travel farther
	SHOCK_WAVE = 1 << 20,

	// x% chance after every attack to spawn a stationary wall of fire, same damage as original attack but can hit multiple times
	// Its Level is the max # of hits + how long it lasts
	FIRE_WALL = 1 << 21, 

	// Grants immunity to and destroys any surge that touches it
	SURGE_BLOCKER = 1 << 22,

	// x% chance to push (small knockback) an enemy on hit (*) (-)
	SHOVE = 1 << 23,

	// x% to deal double damage
	CRITICAL = 1 << 24, 

	// x% chance to survive fatal hit on 1 hp
	SURVIVOR = 1 << 25, 

	// The first time it sees an enemy it will become intangible and invisibile to all hostile-sorces
	// as it walks a set distance, but it also cannot interact with them.
	PHASE = 1 << 26,

	// When it dies, it will spawn a clone of itself at x% health, y distance away, after z seconds
	CLONE = 1ULL << 27,

	// stops an enemy from cloning if dealt the finishing blow (-)
	CODE_BREAKER = 1ULL << 28,

	// leaps over a gap with a width that is <= x units, can do this y times
	LEAP = 1ULL << 29, 

	// jumps to a higher ledge at an open gap with a width that is <= x units, can do this y times
	JUMP = 1ULL << 30,

	// When spawning, start at x% of the lanes distance
	DROP_BOX = 1ULL << 31, 

	// Enemies that are hit with x% health remaining will instantly die 
	TERMINATE = 1ULL << 32,

	// Received knockback force is 50% greater (-)
	LIGHTWEIGHT = 1ULL << 33,

	// Received knockback force is 30% less (-)
	HEAVYWEIGHT = 1ULL << 34, 

	// Unit will Knockback enemies 50% father (*) (-)
	BULLY = 1ULL << 35,

	// Killing # of enemies will summon a Unit
	SALVAGE = 1ULL << 36,

	// When an enemy(boss) dies, they transform into their next phase (-)
	TRANSFORM = 1ULL << 37, 

	// Units releases a damaging explosion after death (-)
	SELF_DESTRUCT = 1ULL << 38, 

	// Unit dies after attacking (-)
	FRAGILE = 1ULL << 39, 

	// Launches a projectile when attacking,
	PROJECTILE = 1ULL << 40, 

	// Projectiles that hit the unit will take extra chip dmg (-)
	ROUGH = 1ULL << 41,

	// When hit with or using a suitable Augment, it will apply the augment effects to Units within x units, and can reach through adjacent lanes if specified.
	// Harmful Augments wil be applied to enemies and Helpful Augments will be applied to allies.
	// Check the get_link_target_team() static Augment function for the list of suitable Augments.
	LINK = 1ULL << 42,

	// Heals up x knockback threshold after killing y enemies
	SYPHON = 1ULL << 43, 

	// Every x seconds, the damage taken over that time period is released as an explosion. (Exclusive to Capcitor types)
	DISCHARGE = 1ULL << 44,

	// Hit enemies ahev an x% chance to be warped back y units. Can be sent up or down lanes. 
	// If sent below all lanes, they fall to their death, if sent above all lanes, they take fall damage of 33% max health + 15% for each lane.
	WARP = 1ULL << 45
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

constexpr AugmentType SURGE_TYPES = AugmentType::ORBITAL_STRIKE | AugmentType::FIRE_WALL | AugmentType::SHOCK_WAVE;
constexpr AugmentType MOBILITY_TYPES = AugmentType::LEAP | AugmentType::JUMP | AugmentType::PHASE;
constexpr AugmentType HEALTH_THRESHOLD_TYPES = AugmentType::STRENGTHEN | AugmentType::VOID | AugmentType::REINFORCE | AugmentType::SCOPE;
constexpr AugmentType NEGATIVE_STATUS_TYPES = AugmentType::SLOW | AugmentType::WEAKEN | AugmentType::OVERLOAD | AugmentType::BLIND | AugmentType::BOLT | AugmentType::SHORT_CIRCUIT | AugmentType::VIRUS | AugmentType::CORRODE | AugmentType::DETONATE;
constexpr AugmentType POSITIVE_STATUS_TYPES = AugmentType::SCOPE | AugmentType::STRENGTHEN | AugmentType::OVERCLOCK | AugmentType::REINFORCE;
constexpr AugmentType STATUS_TYPES = AugmentType::SLOW | AugmentType::WEAKEN | AugmentType::OVERLOAD | AugmentType::BLIND | AugmentType::BOLT | AugmentType::SHORT_CIRCUIT | AugmentType::VIRUS | AugmentType::DETONATE | AugmentType::CORRODE | AugmentType::SCOPE | AugmentType::STRENGTHEN | AugmentType::OVERCLOCK | AugmentType::REINFORCE;
constexpr AugmentType STAT_CHANGE_TYPES = AugmentType::STRENGTHEN | AugmentType::OVERCLOCK | AugmentType::REINFORCE | AugmentType::SCOPE;
constexpr AugmentType DAMAGE_MODIFIER_TYPES = AugmentType::STRENGTHEN | AugmentType::REINFORCE | AugmentType::BREAKER | AugmentType::RESIST;
constexpr AugmentType KILL_REQUIREMENT_TYPES = AugmentType::SALVAGE | AugmentType::SYPHON;
constexpr AugmentType TIMER_TYPES = AugmentType::DISCHARGE | AugmentType::OVERCLOCK;
