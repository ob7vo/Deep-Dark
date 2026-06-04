#pragma once
#include "AugmentTypes.h"
#include <string_view>
#include <json_fwd.hpp>
#include <random>
const int ALL_HITS = 0b11111111;
const int empty = 0;

struct Augment {
public:
	AugmentType augType = AugmentType::NONE;
	// Mask used by units who have multihit attacks
	// The flipped bits are the indexes of what hits the ability can activate
	int activeHits = ALL_HITS;
	float activationChance = 100.f;

	// Union for naming of different abilites to avoid vague names	
    union{
		/// <summary>
		/// Augments that are simple, with just a trigger, maybe some other stats
		/// Includes: CRITICAL, SURVIVOR, VOID, DROP_BOX
		/// </summary>
		struct { float magnitude; float magnitude2;} general;
		//For augments that chagne damages values by percentages against targeted types
 		struct { float dmgMultiplier;} damage; 
		// activates on health threshold
		struct { float buffMagnitude; float hpPercentage; } onHPThreshold; 
		// activates on timer
		struct { float effectMagnitude; float interval; } onTimer; 
        struct { float effectValue; float duration; } status;
        struct { float spawnDistance; int level; } surge;
        struct { int level; float radius; } cannon;
		// For auments that move the Unuit, like JUMP or PHASE
		struct { float distance;} mobility;
        struct { int ID; } projectile;
		// For the LINK Augment
		struct { float range; bool reachesAdjacentLanes;} link;
		// For the DETONATE Augment
		struct { float hpPercentage; float duration; float explosionRange; } detonate; 
		// For Augments that require killing a certain number of enemies to proc, like Salvage
		struct { int effectMagnitude; int requiredKills; int salvageID; } killStreak;
		struct { int ID;} transform;
		struct { float hpPercentage; float duration; float spawnDisplacement; } clone;
		struct { float explosionRange; bool hitsAdjacentLanes; float hpPercentage;} selfDestruct;
		struct { float distance; int laneDisplacemnet; } warp;
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
	static int get_link_target_team(AugmentType augType);

	inline bool is_surge() const { return has(augType, SURGE_TYPES); }

	inline bool is_status_effect() const { return has(augType, STATUS_TYPES); }

	/// <summary>
	/// Augments that deal with movement in some way.
	/// Includes: LEAP, JUMP, PHASE
	/// </summary>
	inline bool is_mobility() const { return has(augType, MOBILITY_TYPES); }
	/// <summary>
	/// Un-Typed Augments that change Unit's stats. Includes: REINFORCE, SCOPE, and STRENGTHEN
	/// </summary>
	inline bool is_stat_change() const { return has(augType, STAT_CHANGE_TYPES); }
	/// <summary>
	/// Augments that need kills to activate, 
	/// </summary>
	inline bool needs_kills() const { return has(augType, KILL_REQUIREMENT_TYPES); }	
	/// <summary>
	/// Included Types: SLOW, OVERLOAD, WEAKEN, BLIND, BOLT, CORRODE, SHORT_CIRCUIT, VIRUS
	/// </summary>
	inline bool is_negative_status() const {return has(augType, NEGATIVE_STATUS_TYPES);}
	/// <summary>
	/// Included Types: OVERCLOCK, REINFORCE, SCOPE, STRENGTHEN
	/// </summary>
	inline bool is_positive_status() const { return has(augType, POSITIVE_STATUS_TYPES); }
	/// <summary>
	/// Augments that trigger after a certain amount of time. Includes only OVERCLOCK & SHOCK_WAVE
	/// </summary>
	inline bool activates_on_timer() const { return has(augType, TIMER_TYPES); }
	/// <summary>
	/// Any Augment that modifies damage values: WEAKEN, CORRODE, BREAKER, RESIST, and STRENGTHEN
	/// </summary>
	inline bool is_damage_modifier() const { return has(augType, DAMAGE_MODIFIER_TYPES); }
	/// <summary>
	/// Includes: STRENGTHEN, REINFORCE, SCOPE, and VOID.
	/// </summary>
	inline bool activates_via_health_threshold() const {return has(augType, HEALTH_THRESHOLD_TYPES); }

	inline bool can_hit(int hitIndex) const {
		return activeHits & (1 << hitIndex);
	}

	bool try_activate(int hitIndex) const;
	bool try_activate() const;

	// All "magnitude" variables are on teh first 4 bytes, so they SHOULD (please) lineupfor this to work
	inline float get_magnitude() const {return data.general.magnitude;}

	static int get_bit_position(AugmentType type) {
		return std::countr_zero(static_cast<size_t>(type));
	}
};
