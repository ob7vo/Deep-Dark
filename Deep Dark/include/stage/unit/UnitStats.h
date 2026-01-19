#pragma once
#include "UnitEnums.h" // Has Augment.h, which has json_fwd.hpp
#include "Augment.h"
#include "json_fwd.hpp"
#include "UnitConfigConstants.h"
#include "SFML/System/Vector2.hpp"

struct Hit {
	int dmg = 0;
	std::pair<int, int> laneReach = { 0,0 };
	std::pair<float, float> attackRange = { 0.f,0.f };

	Hit() = default;
	explicit Hit(int dmg, std::pair<int, int> laneReach = {}, std::pair<float, float> attackRange = {}) :
		dmg(dmg), laneReach(laneReach), attackRange(attackRange) { }
};

// Runtime stats for the Units during Gameplay (Shared data via pointer)
struct UnitStats {
	// * = wont be change by core
	int id = -1; // *
	int gear = 1;
	float rechargeTime = 0.f;

	/// <summary>
	/// Needed for trigger checking on traps, Projectile collision, and 
	/// for falling off gaps (bigger Units falling off on their origin looks ridiculous).
	/// Some checks, like Unit-on-Unit attacks, just use world position, no hurtBoxes
	/// </summary>
	sf::Vector2f hurtBox = {25.f, 30.f};

	int team = 0; // *
	int parts = 0;

	int maxHp = 0;
	int knockbacks = 0;
	float speed = 0;
	float attackTime = 0;

	std::vector<Hit> hits = {};
	int totalHits = 1; // *
	float sightRange = 0;
	std::pair<int, int> laneSight = { 0,0 };
	bool singleTarget = false;

	// these are all bit masks
	UnitType unitTypes = UnitType::NONE;
	UnitType targetTypes = UnitType::NONE;
	AugmentType immunities = AugmentType::NONE;
	AugmentType augmentsMask = AugmentType::NONE;

	std::vector<Augment> augments = {};

	static UnitType convert_string_to_type(std::string_view str);

	void apply_core_modifier(const std::string& core, int gear);
	void removeAugment(AugmentType augType);
	void addCoreAugment(const nlohmann::json& file, const std::string& aug); 
	void modifyDmg(int hitIndex, char op, float value);

	UnitStats() = default;

	void setup(const nlohmann::json& file);

	static UnitStats create_enemy(const nlohmann::json& file, float magnification);
	static UnitStats create_player(const nlohmann::json& file, int core = -1);
	static UnitStats create_cannon(const nlohmann::json& baseFile, float magnification);

	inline const Hit& get_hit_stats(int hitIndex) const { return hits[hitIndex]; }
	inline bool rusted_tyoe() const { return has(unitTypes, UnitType::RUSTED); }
	inline bool ancient_type() const { return has(unitTypes, UnitType::ANCIENT); }
	inline bool floating_type() const { return has(unitTypes, UnitType::FLOATING); }
	inline bool has_surge() const {
		return has(augmentsMask, AugmentType::ORBITAL_STRIKE) ||
			has(augmentsMask, AugmentType::FIRE_WALL) || has(augmentsMask, AugmentType::SHOCK_WAVE);
	}
	inline bool surge_blocker() const { return has(augmentsMask, AugmentType::SURGE_BLOCKER); }
	inline bool is_targeted(UnitType enemyTargetTypes) const { return has(enemyTargetTypes, unitTypes); }
	inline bool targets_unit(UnitType enemyTypes) const { return has(enemyTypes, targetTypes); }

	inline bool is_player() const { return team == 1; }
	inline float get_dir() const { return static_cast<float>(team); }

	/// <summary>
	/// Returns true if the Unit has the correct augment, is on the correct
	/// HitIndex to use it, and successfully rolls the chance for it
	/// </summary>
	bool try_proc_augment(AugmentType target, int hits = 0) const;
	int get_parts_value(const nlohmann::json& json) const;

	inline bool has_augment(AugmentType aug) const { return has(augmentsMask & aug); }
	inline bool has_augment(AugmentType aug, UnitType enemyTypes) 
		const { return has(augmentsMask & aug) && targets_unit(enemyTypes); }
	const Augment* get_augment(AugmentType aug) const;

	inline std::pair<float, float> IDGearPair() const { return { id, gear }; }
};