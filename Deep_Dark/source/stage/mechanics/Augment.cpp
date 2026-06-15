#include "pch.h"
#include "Augment.h"
#include "Utils.h"

/*
Augment::Augment(AugmentType aug, float val, float val2, float percentage,
	int hits, int lvl) :augType(aug), activeHits(hits), value(val), value2(val2),
	percentage(percentage), surgeLevel(lvl) {
}
	*/
Augment Augment::create_status(AugmentType augType, float procTime, float chance, int hits) {
	Augment aug;
	aug.data.status.duration = procTime;
	aug.activationChance = chance;
	aug.activeHits = hits;
	aug.augType = augType;

	return aug;
}
Augment Augment::create_surge(AugmentType augType, float dist, int lvl, float chance, int hits) {
	Augment aug;
	aug.data.surge.spawnDistance = dist;
	aug.data.surge.level = lvl;
	aug.activationChance = chance;
	aug.activeHits = hits;
	aug.augType = augType;

	return aug;
}
Augment Augment::create_cannon(AugmentType augType, int lvl) {
	Augment aug;
	aug.augType = augType;
	aug.data.cannon.level = lvl;
	return aug;
}

Augment Augment::create_from_json(AugmentType augType, const nlohmann::json& augJson) {
	Augment aug = Augment(augType);
	
    if (augJson.contains("active_hits")) {
        aug.activeHits = 0;
        for (int hitIndex : augJson["active_hits"].get<std::vector<int>>())
            aug.activeHits |= (1 << hitIndex);
    }
	
	aug.activationChance = augJson.value("activation_chance", 0.0f);

	// Find what specific type the augment is and fille out its union

	// Start with the specific Augment Unions
	switch (aug.augType) {
	case AugmentType::DETONATE:
		aug.data.detonate.hpPercentage = augJson["hp_percentage"].get<float>();
		aug.data.detonate.explosionRange = augJson["explosion_range"].get<float>();
		aug.data.detonate.duration = augJson["duration"].get<float>();
		return aug;
	case AugmentType::TRANSFORM:
		aug.data.transform.ID = augJson["transform_id"].get<int>();
		return aug;
	case AugmentType::CLONE:
		aug.data.clone.hpPercentageAfterRevival = augJson["hp_percentage_after_revival"].get<float>();
		aug.data.clone.spawnDisplacement = augJson["spawn_displacement"].get<float>();
		aug.data.clone.duration = augJson["duration"].get<float>();
		return aug;
	case AugmentType::SELF_DESTRUCT:
		aug.data.selfDestruct.explosionRange = augJson["explosion_range"].get<float>();
		aug.data.selfDestruct.hitsAdjacentLanes = augJson["hits_adjacent_lanes"].get<bool>();
		aug.data.selfDestruct.hpPercentage = augJson["hp_percentage"].get<float>();
		return aug;
	case AugmentType::PROJECTILE:
		aug.data.projectile.ID = augJson["projectile_id"].get<int>();
		return aug;
	case AugmentType::LINK:
		aug.data.link.range = augJson["range"].get<float>();
		aug.data.link.reachesAdjacentLanes = augJson["reaches_adjacent_lanes"].get<bool>();
		return aug;
	case AugmentType::WARP:
		aug.data.warp.distance = augJson["distance"].get<float>();
		aug.data.warp.laneDisplacemnet = augJson["lane_displacement"].get<int>();
		return aug;
	case AugmentType::CONSTRUCT:
		aug.data.construct.summonID = augJson["summon_id"].get<int>();
		aug.data.construct.requiredKills = augJson["required_kills"].get<int>();
		aug.data.construct.spawnRadius = augJson["spawn_radius"].get<float>();
		return aug;
	case AugmentType::SALVAGE:
		aug.data.salvage.hpPercentageAfterRevival = augJson["hp_percentage_after_revival"].get<float>();
		aug.data.salvage.requiredKills = augJson["required_kills"].get<int>();
		aug.data.salvage.reviveRange = augJson["revive_range"].get<float>();
		return aug;
	case AugmentType::SYPHON:
		aug.data.syphon.restoredKnockbacks = augJson["restored_knockbacks"].get<int>();
		aug.data.syphon.requiredKills = augJson["required_kills"].get<int>();
		return aug;
	}

	// Then check the more general types of Augments
	if (aug.is_negative_status()) {
		aug.data.status.effectValue = augJson["effect_value"].get<float>();
		aug.data.status.duration = augJson["duration"].get<float>();
	}
	else if (aug.activates_on_timer()) {
		aug.data.onTimer.effectMagnitude = augJson["effect_magnitude"].get<float>();
		aug.data.onTimer.interval = augJson["interval"].get<float>();
	}
	else if (aug.activates_via_health_threshold()) {
		aug.data.onHPThreshold.hpPercentage = augJson["health_threshold_percentage"].get<float>();
		aug.data.onHPThreshold.buffMagnitude = augJson["buff_magnitude"].get<float>();
	}
	else if (aug.is_damage_modifier()) {
		aug.data.damage.dmgMultiplier = augJson["damage_multiplier"].get<float>();
	}
	else if (aug.is_surge()){
		aug.data.surge.spawnDistance = augJson.value("spawn_distance", 0.0f);
		aug.data.surge.level = augJson["surge_level"].get<int>();
	}
	else if (aug.is_mobility()) {
		aug.data.mobility.distance = augJson["distance"].get<float>();
	}
	else {
		aug.data.general.magnitude = augJson["magnitude"].get<float>();
    	aug.data.general.magnitude2 = augJson.value("magnitude2", 0.0f);
	}
}
AugmentType Augment::string_to_augment_type(std::string_view strView) {
	std::string str(strView);
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);

	static const std::unordered_map<std::string, AugmentType> augmentMap = {
		{"none", AugmentType::NONE},
		{"slow", AugmentType::SLOW},
		{"overload", AugmentType::OVERLOAD},
		{"weaken", AugmentType::WEAKEN},
		{"blind", AugmentType::BLIND},
		{"corrode", AugmentType::CORRODE},
		{"short_circuit", AugmentType::SHORT_CIRCUIT},
		{"virus", AugmentType::VIRUS},
		{"overclock", AugmentType::OVERCLOCK},
		{"reinforce", AugmentType::REINFORCE},
		{"scope", AugmentType::SCOPE},
		{"breaker", AugmentType::BREAKER},
		{"resist", AugmentType::RESIST},
		{"strengthen", AugmentType::STRENGTHEN},
		{"squash", AugmentType::SQUASH},
		{"launch", AugmentType::LAUNCH},
		{"plunder", AugmentType::PLUNDER},
		{"void", AugmentType::VOID},
		{"orbital_strike", AugmentType::ORBITAL_STRIKE},
		{"shock_wave", AugmentType::SHOCK_WAVE},
		{"fire_wall", AugmentType::FIRE_WALL},
		{"surge_blocker", AugmentType::SURGE_BLOCKER},
		{"shove", AugmentType::SHOVE},
		{"critical", AugmentType::CRITICAL},
		{"survivor", AugmentType::SURVIVOR},
		{"phase", AugmentType::PHASE},
		{"clone", AugmentType::CLONE},
		{"salvage", AugmentType::SALVAGE},
		{"construct", AugmentType::CONSTRUCT },
		{"code_breaker", AugmentType::CODE_BREAKER},
		{"leap", AugmentType::LEAP},
		{"jump", AugmentType::JUMP},
		{"drop_box", AugmentType::DROP_BOX},
		{"terminate", AugmentType::TERMINATE},
		{"lightweight", AugmentType::LIGHTWEIGHT},
		{"heavyweight", AugmentType::HEAVYWEIGHT},
		{"bully", AugmentType::BULLY},
		{"fragile", AugmentType::FRAGILE},
		{"self_destruct", AugmentType::SELF_DESTRUCT},
		{"projectile", AugmentType::PROJECTILE},
		{"rough", AugmentType::ROUGH},
		{"link", AugmentType::LINK},
		{"syphon", AugmentType::SYPHON},
		{"discharge", AugmentType::DISCHARGE},
		{"detonate", AugmentType::DETONATE},
		{"transform", AugmentType::TRANSFORM},
		{"warp", AugmentType::WARP}
	};

	auto it = augmentMap.find((std::string)str);
	if (it == augmentMap.end())
		std::cout << "Invalid Augment String: [" << str << "]" << std::endl;
	return (it != augmentMap.end()) ? it->second : AugmentType::NONE;
}
int Augment::get_link_target_team(AugmentType augType) {
	static const std::unordered_set<AugmentType> linkToEnemies = {
		AugmentType::SLOW, 
		AugmentType::OVERLOAD,
		AugmentType::WEAKEN,
		AugmentType::BLIND,
		AugmentType::CORRODE,
		AugmentType::SHORT_CIRCUIT,
		AugmentType::VIRUS,
		AugmentType::DETONATE,
		AugmentType::BOLT,
		AugmentType::SHOVE
	};
	static const std::unordered_set<AugmentType> linkToAllies = {
		AugmentType::OVERCLOCK,
		AugmentType::REINFORCE,
		AugmentType::SCOPE,
		AugmentType::STRENGTHEN,
		AugmentType::SYPHON
	};

	if (linkToAllies.contains(augType)) return 1;
	else if (linkToEnemies.contains(augType)) return -1;
	else return 0;
}

bool Augment::try_activate(int hitIndex) const {
	return can_hit(hitIndex) && Random::chance(activationChance);
}
bool Augment::try_activate() const {
	return Random::chance(activationChance);
}