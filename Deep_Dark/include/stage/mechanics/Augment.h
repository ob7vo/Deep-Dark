#pragma once
#include "AugmentTypes.h"
#include <string_view>
#include <json_fwd.hpp>

const int ALL_HITS = 0b11111111;
const int empty = 0;

struct Augment {
public:
	AugmentType augType = AugmentType::NONE;
	// Mask used by units who have multihit attacks
	// The flipped bits are the indexes of what hits the ability can activate
	int activeHits = ALL_HITS;
	float activationChance;
	// Union for naming of different abilites to avoid vague names	
    union{
		struct { float magnitude; float magnitude2;} general;
 		struct { float dmgMultiplier;} damage; //For augments that chagne damages values by percentages against targeted types
		struct { float buffMagnitude; float hpPercentage; } onHPThreshold; // activates on health threshold
		struct { float effectMagnitude; float elapsed; } onTimer; // activates on timer
        struct { float effectValue; float duration; } status;
        struct { float spawnDistance; int level; } surge;
		struct { float hp;} sheild;
        struct { int level; float radius; } cannon;
		struct { float distance;} mobility; // For auments that move the Unuit, like JUMP or PHASE
        struct { float speed; float id; } projectile;
		struct { int effectMagnitude; int requiredKills; } killStreak; // For Augments that require killing a certain number of enemies to proc, like Salvage
		struct { int id;} unitSpawn;
		struct { int range; int size;} selfDestruct;
    } data;

	Augment() = default;
	Augment(AugmentType type) : augType(type) {}

	static Augment create_from_json(AugmentType augType, const nlohmann::json& json);
	static Augment create_status(AugmentType aug, float procTime, float chance = 100.f, int hits = ALL_HITS);
	static Augment create_surge(AugmentType aug, float dist, int lvl, float chance = 100.f, int hits = ALL_HITS);
	static Augment create_cannon(AugmentType aug, int lvl);
	static AugmentType string_to_augment_type(std::string_view str);
	/// <summary>
	/// Checks if a linking augmetn targets enemies or allies
	/// </summary>
	/// <returns>
	/// - 0 if the Augment is not supported by "LINK".
	/// - If it's supported, returns 1 to target allies, and -1 for enemies
	/// </returns>
	static int links_to_allies(AugmentType augType);

	inline bool is_surge() const {  
		return has(augType & AugmentType::ORBITAL_STRIKE) ||
			has(augType & AugmentType::FIRE_WALL) || has(augType & AugmentType::SHOCK_WAVE);
	}
	inline bool is_status_effect() const { return augType <= AugmentType::SCOPE && augType > AugmentType::NONE; }
	inline bool is_mobility() const { return has(augType & AugmentType::LEAP) || has(augType & AugmentType::JUMP) || has(augType & AugmentType::PHASE); }
	inline bool is_stat_change() const { return augType >= AugmentType::STRENGTHEN && augType <= AugmentType::SUPERIOR; }
	inline bool needs_kills() const { return has(augType & AugmentType::SALVAGE) && has(augType & AugmentType::SYPHON); }
	inline bool is_negative_status() const {return augType <= AugmentType::VIRUS && augType > AugmentType::NONE;}
	inline bool is_positive_status() const { return augType >= AugmentType::OVERCLOCK && augType <= AugmentType::SCOPE; }
	inline bool is_damage_modifier() const { return augType >= AugmentType::STRENGTHEN && augType <= AugmentType::SUPERIOR; }
	inline bool triggers_on_hit() const {return true;}
	inline bool activates_via_health_threshold() const {return augType >= AugmentType::REINFORCE && augType <= AugmentType::STRENGTHEN;}
	inline bool can_hit(int hitIndex) const {
		return activeHits & (1 << hitIndex);
	}

	// All "magnitude" variables are on teh first 4 bytes, so they SHOULD (please) lineupfor this to work
	inline float get_magnitude() const {return data.general.magnitude;}

	static int get_bit_position(AugmentType type) {
		return std::countr_zero(static_cast<size_t>(type));
	}
};
