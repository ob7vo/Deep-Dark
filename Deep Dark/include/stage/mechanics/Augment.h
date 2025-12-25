#pragma once
#include "AugmentTypes.h"
#include <json_fwd.hpp>

const int ALL_HITS = 0b11111111;
const int empty = 0;

struct Augment {
	AugmentType augType = AugmentType::NONE;
	// Mask used by units who have multihit attacks
	// The flipped bits are the indexes of what hits the ability can activate
	int activeHits = ALL_HITS;
	/// <summary> Duration of status effects or distance placements </summary>
	float value = 0.0f;
	float value2 = 0.0f;
	union {
		// In Union with value3
		float percentage = 1.0f;
		// In Union with percentage
		float value3;
	};
	union {
		// In Union with intValue
		int surgeLevel = 1;
		// In Union with surgeLevel
		int intValue;
	};

	Augment() = default;
	Augment(AugmentType aug, float val, float val2, float percentage,
		int hits, int lvl);

	static Augment from_json(AugmentType augType, const nlohmann::json& json);
	static Augment status(AugmentType aug, float procTime, float chance = 100.f, int hits = ALL_HITS);
	static Augment surge(AugmentType aug, float dist, int lvl, float chance = 100.f, int hits = ALL_HITS);
	static Augment cannon(AugmentType aug, int lvl);
	static AugmentType string_to_augment_type(std::string_view str);

	inline bool is_surge() const {
		return has(augType & AugmentType::ORBITAL_STRIKE) ||
			has(augType & AugmentType::FIRE_WALL) || has(augType & AugmentType::SHOCK_WAVE);
	}
	inline bool is_status_effect() const { return augType <= AugmentType::VIRUS && augType > AugmentType::NONE; }
	inline bool is_damage_modifier() const { return augType >= AugmentType::STRENGTHEN && augType <= AugmentType::SUPERIOR; }
	inline bool can_hit(int hitIndex) const {
		return activeHits & (1 << hitIndex);
	}

	static int get_bit_position(AugmentType type) {
		return std::countr_zero(static_cast<size_t>(type));
	}
};
