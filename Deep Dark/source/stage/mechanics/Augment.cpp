#include "pch.h"
#include "Augment.h"

Augment::Augment(AugmentType aug, float val, float val2, float percentage,
	int hits, int lvl) :augType(aug), activeHits(hits), value(val), value2(val2),
	percentage(percentage), surgeLevel(lvl) {
}
Augment Augment::status(AugmentType aug, float procTime, float chance, int hits) {
	return Augment(aug, procTime, empty, chance, hits, empty);
}
Augment Augment::surge(AugmentType aug, float dist, int lvl, float chance, int hits) {
	return Augment(aug, dist, empty, chance, hits, lvl);
}
Augment Augment::cannon(AugmentType aug, int lvl) {
	return Augment(aug, empty, empty, empty, ALL_HITS, lvl);
}

Augment Augment::from_json(AugmentType augType, const nlohmann::json& augJson) {
    float val = augJson.value("value", 0.0f);
    float val2 = augJson.value("value2", 0.0f);
    float percentage = augJson.value("percentage", 0.0f);
	percentage = augJson.value("value3", percentage);

    int activeHits = ALL_HITS;
    if (augJson.contains("active_hits")) {
        activeHits = 0;
        for (int hitIndex : augJson["active_hits"])
            activeHits |= (1 << hitIndex);
    }
	 
    int lvl = augJson.value("surge_level", 1);
    lvl = augJson.value("int_value", lvl);

    return Augment(augType, val, val2, percentage, activeHits, lvl);
}
AugmentType Augment::string_to_augment_type(std::string_view str) {
	static const std::unordered_map<std::string, AugmentType> augmentMap = {
		{"none", AugmentType::NONE},
		{"slow", AugmentType::SLOW},
		{"overload", AugmentType::OVERLOAD},
		{"weaken", AugmentType::WEAKEN},
		{"blind", AugmentType::BLIND},
		{"corrode", AugmentType::CORRODE},
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
		{"jump", AugmentType::JUMP},
		{"drop_box", AugmentType::DROP_BOX},
		{"warp", AugmentType::WARP},
		{"terminate", AugmentType::TERMINATE},
		{"lightweight", AugmentType::LIGHTWEIGHT},
		{"heavyweight", AugmentType::HEAVYWEIGHT},
		{"bully", AugmentType::BULLY},
		{"salvage", AugmentType::SALVAGE},
		{"fragile", AugmentType::FRAGILE},
		{"self_destruct", AugmentType::SELF_DESTRUCT},
		{"projectile", AugmentType::PROJECTILE},
		{"deflect", AugmentType::DEFLECT},
		{"rough", AugmentType::ROUGH}
	};

	auto it = augmentMap.find((std::string)str);
	if (it == augmentMap.end())
		std::cout << "Invalid Augment String: [" << str << "]" << std::endl;
	return (it != augmentMap.end()) ? it->second : AugmentType::NONE;
}