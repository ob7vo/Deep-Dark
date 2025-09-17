#pragma once
#include "ActionStates.h"
#include "Animation.h"
#include <iostream>
#include <memory>

const int TEAM_PLAYER = 1;
const int TEAM_ENEMY = -1;
const float GUARANTEED_CHANCE = 100.0f;

struct UnitStats {
	int team = 0;
	int parts = 0;

	int maxHp = 0;
	int knockbacks = 0;
	int dmg = 0;
	float speed = 0;
	float attackTime = 0;

	float sightRange = 0;
	float maxAttackRange = 0;
	float minAttackRange = 0;
	int minLaneReach = 0;
	int maxLaneReach = 0;
	bool singleTarget = 0;

	int unitTypes = 0;
	int targetTypes = 0;
	int immunities = 0;
	int quickAugMask = 0;
	std::vector<Augment> augments;	

	std::unique_ptr<Animation> specialAnimation = nullptr;

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
		if (it == typeMap.end()) std::cout << "Invalid UnitType String" << std::endl;

		return (it != typeMap.end()) ? it->second : UnitType::NULL_TYPE;
	}
	static AugmentType convert_string_to_augment(std::string str) {
		std::transform(str.begin(), str.end(), str.begin(), ::tolower);

		static const std::unordered_map<std::string, AugmentType> augmentMap = {
			{"none", AugmentType::NONE},
			{"slow", AugmentType::SLOW},
			{"overload", AugmentType::OVERLOAD},
			{"weaken", AugmentType::WEAKEN},
			{"blind", AugmentType::BLIND},
			{"short_circuit", AugmentType::SHORT_CIRCUIT},
			{"virus", AugmentType::VIRUS},
			{"breaker", AugmentType::BREAKER},
			{"resist", AugmentType::RESIST},
			{"strengthen", AugmentType::STRENGTHEN},
			{"squash", AugmentType::SQUASH},
			{"launch", AugmentType::LAUNCH},
			{"plunder", AugmentType::PLUNDER},
			{"void", AugmentType::VOID},
			{"superior", AugmentType::SUPERIOR},
			{"orbital_strike", AugmentType::ORBITAL_STRIKE},
			{"shock_wave", AugmentType::SHOCK_WAVE},
			{"fire_wall", AugmentType::FIRE_WALL},
			{"surge_blocker", AugmentType::SURGE_BLOCKER},
			{"death_surge", AugmentType::DEATH_SURGE},
			{"counter_surge", AugmentType::COUNTER_SURGE},
			{"shove", AugmentType::SHOVE},
			{"shield", AugmentType::SHIELD},
			{"shield_pierce", AugmentType::SHIELD_PIERCE},
			{"critical", AugmentType::CRITICAL},
			{"survivor", AugmentType::SURVIVOR},
			{"phase", AugmentType::PHASE},
			{"clone", AugmentType::CLONE},
			{"code_breaker", AugmentType::CODE_BREAKER},
			{"leap", AugmentType::LEAP},
			{"jump", AugmentType::JUMP}
		};

		auto it = augmentMap.find(str);
		if (it == augmentMap.end()) std::cout << "Invalid Augment String" << std::endl;
		return (it != augmentMap.end()) ? it->second : AugmentType::NONE;
	}

	UnitStats() : augments{} {}
	UnitStats(const nlohmann::json& file, float magnification) : augments{} {
		if (file["animations"].contains("special")) {
			bool loops = file["animations"]["special"].value("loops", true);
			specialAnimation = std::make_unique<Animation>(Animation::create_unit_animation(file, "special", loops));
			std::cout << "creating special animation. Address: " << specialAnimation << std::endl;
		}

		team = file["team"];	
		if (team == TEAM_PLAYER) parts = file["parts_cost"];
		else parts = file["parts_dropped"];
		unitTypes = targetTypes = immunities = quickAugMask = 0;
		unitTypes |= UnitType::ALL;
		augments.reserve(3);

		for (auto& type : file["types"])
			unitTypes |= convert_string_to_type(type);
		if (file.contains("target_types"))
			for (auto& target : file["target_types"])
				targetTypes |= convert_string_to_type(target);
		if (file.contains("immunities"))
			for (auto& immunity : file["immunities"])
				immunities |= convert_string_to_augment(immunity);
		if (file.contains("quick_augment_masks"))
			for (auto& quick : file["quick_augment_masks"])
				quickAugMask |= convert_string_to_augment(quick);

		if (file.contains("augments")) {
			for (auto& aug : file["augments"]) {
				AugmentType augType = convert_string_to_augment(aug["augment"]);
				if (augType == NONE) continue;
				quickAugMask |= augType;

				float value = aug.value("value", 0.0f);
				float percentage = aug.value("percentage", 0.0f);
				int lvl = aug.value("surge_level", 1);

				augments.emplace_back(augType, value, percentage, lvl);
			}
		}

		maxHp = static_cast<int>(file["stats"]["hp"] * magnification);
		dmg = static_cast<int>(file["stats"]["dmg"] * magnification);
		knockbacks = static_cast<int>(file["stats"]["knockbacks"]);
		speed = file["stats"]["speed"];
		attackTime = file["stats"]["attack_time"];
		sightRange = file["stats"]["sight_range"];
		maxAttackRange = file["stats"]["max_attack_range"];
		minAttackRange = file["stats"]["min_attack_range"];
		minLaneReach = file["stats"]["min_lane_reach"];
		maxLaneReach = file["stats"]["max_lane_reach"];
		singleTarget = file["single_target"];
		parts = file.value("parts", 0);
	}
	static UnitStats create_cannon(const nlohmann::json& baseFile, float magnification) {
		UnitStats stats;
		stats.team = baseFile["team"];
		stats.targetTypes = convert_string_to_type(baseFile["target_type"]);
		stats.dmg = static_cast<int>(std::round(baseFile.value("dmg", 1) * magnification));

		if (baseFile.contains("augment")) {
			AugmentType aug = convert_string_to_augment(baseFile["augment"]["augment_type"]);
			float procTime = baseFile["augment"].value("status_time", 1.f);
			stats.augments.emplace_back(aug, procTime, GUARANTEED_CHANCE);
		}

		return stats;
	}

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
	inline Augment get_augment(AugmentType aug) const {
		for (auto& augment : augments)
			if (augment.augType == aug) return augment;

		return {};
	}
};

