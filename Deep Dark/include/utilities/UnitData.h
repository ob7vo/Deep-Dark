#pragma once
#include "UnitEnums.h"
#include "SFML/Graphics/Texture.hpp"
#include <iostream>
#include <fstream>
#include <memory>

const int TEAM_PLAYER = 1;
const int TEAM_ENEMY = -1;
const float GUARANTEED_CHANCE = 100.0f;


namespace UnitData {
	const int TOTAL_PLAYER_UNITS = 3;
	const int TOTAL_ENEMY_UNITS = 4;
	const int TOTAL_UNITS = TOTAL_PLAYER_UNITS + TOTAL_ENEMY_UNITS;

	const int NULL_ID = -1;

	std::string get_unit_folder_path(int id, int gear = 0);
	nlohmann::json get_unit_json(int id, int gear = 0);
	sf::Texture get_slot_texture(int id, int gear);

	std::string get_unit_folder_path(std::pair<int,int> unit);
	nlohmann::json get_unit_json(std::pair<int, int> unit);
	sf::Texture get_slot_texture(std::pair<int, int> unit);
};

struct Hit {
	int dmg = 0;
	std::pair<int, int> laneReach = { 0,0 };
	std::pair<float, float> attackRange = { 0.f,0.f };

	Hit() = default;
	Hit(int dmg, std::pair<int, int> laneReach = {}, std::pair<float, float> attackRange = {}) :
		dmg(dmg), laneReach(laneReach), attackRange(attackRange) { }
};

struct UnitStats {
	// * = wont be change by core
	int unitId = 0; // *
	float rechargeTime = 0.f;

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

	static UnitType convert_string_to_type(std::string str) {
		std::transform(str.begin(), str.end(), str.begin(), ::tolower);

		static const std::unordered_map<std::string, UnitType> typeMap = {
			{"typeless", UnitType::TYPELESS},
			{"steel", UnitType::STEEL},
			{"nano", UnitType::NANO},
			{"rusted", UnitType::RUSTED},
			{"floating", UnitType::FLOATING},
			{"reanimen", UnitType::REANIMEN},
			{"ancient", UnitType::ANCIENT},
			{"hologram", UnitType::HOLOGRAM},
			{"voided", UnitType::VOIDED},
			{"all", UnitType::ALL}
		};

		auto it = typeMap.find(str);
		if (it == typeMap.end())
			std::cout << "Invalid UnitType String: [" << str << "]" << std::endl;

		return (it != typeMap.end()) ? it->second : UnitType::NULL_TYPE;
	}

	static const std::unordered_map<std::string, std::function<void(UnitStats&, char, const std::string&)>> statModifiers;
	void apply_core_modifier(const std::string& core, int gear);
	void removeAugment(AugmentType augType);
	void addCoreAugment(const nlohmann::json& file, const std::string& aug);
	void modifyDmg(int hitIndex, char op, float value);

	UnitStats() = default;

	void setup(const nlohmann::json& file) {
		std::pair<int, int> baseRange = { 0,0 };

		rechargeTime = file.value("recharge_timer", 0.f);
		unitId = file["unit_id"];
		team = file["team"];
		if (team == TEAM_PLAYER) parts = file["parts_cost"];
		else parts = file["parts_dropped"];

		unitTypes = targetTypes = 0;
		immunities = quickAugMask = 0;
		unitTypes |= UnitType::ALL;
		augments.reserve(3);

		for (auto& type : file["types"])
			unitTypes |= convert_string_to_type(type);
		if (file.contains("target_types"))
			for (auto& target : file["target_types"])
				targetTypes |= convert_string_to_type(target);
		if (file.contains("immunities"))
			for (auto& immunity : file["immunities"])
				immunities |= Augment::string_to_augment_type(immunity);
		if (file.contains("quick_augment_masks"))
			for (auto& quick : file["quick_augment_masks"])
				quickAugMask |= Augment::string_to_augment_type(quick);

		if (file.contains("augments")) {
			for (auto& augJson : file["augments"]) {
				AugmentType augType = Augment::string_to_augment_type(augJson["augment"]);
				if (augType == NONE) continue;
				quickAugMask |= augType;
				augments.emplace_back(Augment::from_json(augType, augJson));
			}
		}

		maxHp = file["stats"]["hp"];
		knockbacks = file["stats"]["knockbacks"];
		speed = file["stats"]["speed"];
		attackTime = file["stats"]["attack_time"];
		sightRange = file["stats"]["sight_range"];
		laneSight = file["stats"].value("lane_sight", baseRange);
		singleTarget = file["single_target"];
		parts = get_parts_value(file);

		if (!file["stats"].contains("hits")) {
			int dmg = file["stats"]["dmg"];
			std::pair<int, int> laneReach = file["stats"].value("lane_reach", baseRange);
			std::pair<float, float> attackRange = file["stats"]["attack_range"];
			hits.emplace_back(dmg, laneReach, attackRange);
		}
		else {
			for (auto& hit : file["stats"]["hits"]) {
				int dmg = hit["dmg"];
				std::pair<int, int> laneReach = hit.value("lane_reach", baseRange);
				std::pair<float, float> attackRange = hit["attack_range"];
				hits.emplace_back(dmg, laneReach, attackRange);
			}
			totalHits = (int)hits.size();
		}
	}
	static UnitStats enemy(const nlohmann::json& file, float magnification);
	static UnitStats player(const nlohmann::json& file, int core = -1);

	static UnitStats create_cannon(const nlohmann::json& baseFile, float magnification) {
		UnitStats stats;
		stats.team = baseFile["team"];
		stats.targetTypes = convert_string_to_type(baseFile["target_type"]);
		auto dmg = static_cast<int>(std::round(baseFile.value("dmg", 1.f) * magnification));
		stats.hits.emplace_back(dmg);

		if (baseFile.contains("augment")) {
			AugmentType aug = Augment::string_to_augment_type(baseFile["augment"]["augment_type"]);
			float procTime = baseFile["augment"].value("status_time", 1.f);
			stats.augments.emplace_back(Augment::status(aug, procTime));
		}

		return stats;
	}

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

	inline Augment get_augment(AugmentType aug) const {
		for (auto& augment : augments)
			if (augment.augType == aug) return augment;

		return {};
	}
	inline int get_parts_value(const nlohmann::json& json) const {
		int p = json.value("parts_dropped", 0);
		p = json.value("parts_cost", p);
		p = json.value("parts", p);
		return p;
	}
};