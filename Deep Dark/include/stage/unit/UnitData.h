#pragma once
#include "UnitEnums.h" // Has Augment.h, which has json_fwd.hpp
#include <json_fwd.hpp>

const int TEAM_PLAYER = 1;
const int TEAM_ENEMY = -1;
const float GUARANTEED_CHANCE = 100.0f;

namespace UnitData {
	const int TOTAL_PLAYER_UNITS = 3;
	const int TOTAL_ENEMY_UNITS = 4;
	const int TOTAL_UNITS = TOTAL_PLAYER_UNITS + TOTAL_ENEMY_UNITS;

	const int NULL_ID = -1;

	std::string get_unit_folder_path(int id, int gear = 1);
	nlohmann::json createUnitJson(int id, int gear = 1);
	sf::Texture createSlotTexture(int id, int gear);
	/// <summary> Gets the highest gear leel Unit #id has  </summary>
	int getMaxGear(int id);

	std::string get_unit_folder_path(std::pair<int,int> unit);
	nlohmann::json createUnitJson(std::pair<int, int> unit);
	sf::Texture createSlotTexture(std::pair<int, int> unit);
};

struct Hit {
	int dmg = 0;
	std::pair<int, int> laneReach = { 0,0 };
	std::pair<float, float> attackRange = { 0.f,0.f };

	Hit() = default;
	explicit Hit(int dmg, std::pair<int, int> laneReach = {}, std::pair<float, float> attackRange = {}) :
		dmg(dmg), laneReach(laneReach), attackRange(attackRange) { }
};

struct UnitStats {
	// * = wont be change by core
	int unitId = 0; // *
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
	int unitTypes = 0;
	int targetTypes = 0;
	size_t immunities = 0;
	size_t quickAugMask = 0; 

	std::vector<Augment> augments = {};

	static UnitType convert_string_to_type(std::string str);

	static const std::unordered_map<std::string, std::function<void(UnitStats&, char, const std::string&)>> statModifiers;
	void apply_core_modifier(const std::string& core, int gear);
	void removeAugment(AugmentType augType);
	void addCoreAugment(const nlohmann::json& file, const std::string& aug);
	void modifyDmg(int hitIndex, char op, float value);

	UnitStats() = default;

	void setup(const nlohmann::json& file);

	static UnitStats enemy(const nlohmann::json& file, float magnification);
	static UnitStats player(const nlohmann::json& file, int core = -1);
	static UnitStats create_cannon(const nlohmann::json& baseFile, float magnification);

	inline const Hit& get_hit_stats(int hitIndex) const { return hits[hitIndex]; }
	inline bool rusted_tyoe() const { return unitTypes & UnitType::RUSTED; }
	inline bool ancient_type() const { return unitTypes & UnitType::ANCIENT; }
	inline bool floating_type() const { return unitTypes & UnitType::FLOATING; }
	inline bool has_surge() const {
		return quickAugMask & AugmentType::ORBITAL_STRIKE ||
			quickAugMask & AugmentType::FIRE_WALL || quickAugMask & AugmentType::SHOCK_WAVE;
	}
	inline bool has_augment(AugmentType aug) const { return quickAugMask & aug; }
	inline bool surge_blocker() const { return quickAugMask & AugmentType::SURGE_BLOCKER; }
	inline bool targeted_by_unit(int enemyTargetTypes) const { return (enemyTargetTypes & unitTypes); }

	inline bool is_player() const { return team == 1; }
	inline float get_dir() const { return static_cast<float>(team); }

	bool try_proc_augment(AugmentType target, int hits = 0) const;
	Augment get_augment(AugmentType aug) const;
	int get_parts_value(const nlohmann::json& json) const;
};