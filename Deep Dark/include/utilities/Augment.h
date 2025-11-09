#pragma once
#include "json.hpp"
#include <bit>

const enum AugmentType : size_t {
	NONE = 0,
	SLOW = 1 << 0, // speed is dropped to 0.1 (traits)
	OVERLOAD = 1 << 1, // speed and all animations are at half speed (deltaTime / 2) (traits)
	WEAKEN = 1 << 2, // lowers damage (traits)
	BLIND = 1 << 3, // halves sight range (traits)
	CORRODE = 1 << 4, // doubles damage taken (traited)
	SHORT_CIRCUIT = 1 << 5, // nullifies trait-augments (curse)
	VIRUS = 1 << 6, // nullfies immunities (traits)
	STRENGTHEN = 1 << 7, // 2x damage and 1/2 damage taken (traits)
	BREAKER = 1 << 8, // Deal 3x damage (traits)
	RESIST = 1 << 9, // take 1/4 damage (traits)
	SUPERIOR = 1 << 10, // deal x% more damage when under y% health
	PLUNDER = 1 << 11, // Enemy units drop twice the parts on death
	VOID = 1 << 12, // cut 30% of current hp (traits)
	SQUASH = 1 << 13, // knockbacked enemies will be sent DOWN a lane (through gaps)
	LAUNCH = 1 << 14, // knockbacked enemies will be sent UP a lane (through gaps)
	ORBITAL_STRIKE = 1 << 15, // spawns one laser directly on an attack enemy, hits all lanes, same damage as original attack, and has a width
	SHOCK_WAVE = 1 << 16, // same as battle cats waves, wave blocker = immunity to wave + last bit flipped
	FIRE_WALL = 1 << 17, // same as battle cats surges
	SURGE_BLOCKER = 1 << 18,
	DEATH_SURGE = 1 << 19, // x% to spawn a surge on death, Augment's percentage determines type
	COUNTER_SURGE = 1 << 20,
	SHOVE = 1 << 21, // small knockback (traits)
	SHIELD = 1 << 22, // same as Aku shields
	SHIELD_PIERCE = 1 << 23,
	CRITICAL = 1 << 24, // x% to deal dpuble damage
	SURVIVOR = 1 << 25, // x% chance to survive fatal hit on 1 hp
	PHASE = 1 << 26, // same as zombie burrow
	CLONE = 1 << 27, // same as zombie revive
	CODE_BREAKER = 1 << 28, // stops cloning
	LEAP = 1 << 29, // leaps distance when approaching a game, can do this y times
	JUMP = 1 << 30, // jumps to a higher ledge at an open gap, can do this x times
	DROP_BOX = 1ULL << 31, // spawn at x% lane completion
	WARP = 1ULL << 32,
	TERMINATE = 1ULL << 33, // Hit enemys with x% health remaining will die
	LIGHTWEIGHT = 1ULL << 34, // Gets knockedback 1.5x farther
	HEAVYWEIGHT = 1ULL << 35, // Gets knockedback .7x farther (or SHORTER i guess, can't be launched
	BULLY = 1ULL << 36, // Knocksback enemies 1.5x farther
	SALVAGE = 1ULL << 37, // Killing an enemy will summon a Unit
	SELF_DESTRUCT = 1ULL << 38, // Units dies after attacking
	PROJECTILE = 1ULL << 39, // Launches a projectile when attacking,
	DEFLECT = 1ULL << 40, // x% chance to deflect (DEFLECTABLE) projectiles
	CHIP = 1ULL << 41 // Projectiles that hit the unit will take extra chip dmg
};

const int ALL_HITS = 0b11111111;
const int empty = 0;

struct Augment {
	AugmentType augType = NONE;
	int activeHits = ALL_HITS;
	/// <summary> Duration of status effects or distance of surge placement </summary>
	float value = 0.0f;
	float value2 = 0.0f;
	union {
		float percentage = 1.0f;
		float value3;
	};
	union {
		int surgeLevel = 1;
		int intValue;
	};

	Augment() = default;
	Augment(AugmentType aug, float val, float val2, float percentage,
		int hits, int lvl) :augType(aug), value(val), value2(val2),
		percentage(percentage), surgeLevel(lvl), activeHits(hits) {
	}

	static Augment from_json(AugmentType augType, nlohmann::json json);
	static Augment status(AugmentType aug, float procTime, float chance = 100.f, int hits = ALL_HITS) {
		return Augment(aug, procTime, empty, chance, hits, empty);
	}
	static Augment surge(AugmentType aug, float dist, int lvl, float chance = 100.f, int hits = ALL_HITS) {
		return Augment(aug, dist, empty, chance, hits, lvl);
	}
	static Augment cannon(AugmentType aug, int lvl) {
		return Augment(aug, empty, empty, empty, ALL_HITS, lvl);
	}
	static AugmentType string_to_augment_type(std::string str);

	inline bool is_surge() const {
		return augType & AugmentType::ORBITAL_STRIKE ||
			augType & AugmentType::FIRE_WALL || augType & AugmentType::SHOCK_WAVE;
	}
	inline bool is_status_effect() const { return augType <= 64 && augType > 0; }
	inline bool is_damage_modifier() const { return augType >= 128 && augType <= 512; }
	inline bool can_hit(int hitIndex) const {
		bool canHit = activeHits & (1 << hitIndex);
		//if (canHit) std::cout << "you CAN proc this augment" << std::endl;
		//else std::cout << "YOU CANNOT PROC THIS AUGMENT BABY" << std::endl;

		return canHit;
	}

	static int get_bit_position(AugmentType type) {
		return std::countr_zero(static_cast<size_t>(type));
	}
};
