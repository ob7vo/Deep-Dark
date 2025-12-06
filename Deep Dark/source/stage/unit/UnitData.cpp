#include "pch.h"
#include "UnitData.h"
#include "Utils.h"


UnitType UnitStats::convert_string_to_type(std::string str) {
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

namespace UnitData {
    std::string get_unit_folder_path(int id, int gear) {
        if (gear < 1 || gear > 3) {
            std::string error = std::format("Gear must be between 1 and 3: The gear was: #{}", gear);
            throw std::runtime_error(error);
        }

        return std::format("configs/unit_data/{}/gear{}/", id, gear);
    }
    nlohmann::json createUnitJson(int id, int gear) {
        const std::string path = get_unit_folder_path(id, gear) + "unit_data.json";
        std::ifstream file(path);

        try {
            return nlohmann::json::parse(file);
        }
        catch (const nlohmann::json::parse_error& e) {
            std::cerr << "JSON parse error in " << path << ": " << e.what() << std::endl;
            return nlohmann::json();
        }
    }
    sf::Texture createSlotTexture(int id, int gear) {
        const std::string path = id >= 0 ? get_unit_folder_path(id, gear) + "slot.png" :
            "sprites/defaults/empty_slot.png";

        sf::Texture slotTex;

        if (!slotTex.loadFromFile(path)) {
            (void)slotTex.loadFromFile("sprites/defaults/empty_slot.png");
        }

        return slotTex;
    }
    int getMaxGear(int id) {
        return 1 + std::filesystem::exists(get_unit_folder_path(id, 2))
            + std::filesystem::exists(get_unit_folder_path(id, 3));
    }

    std::string get_unit_folder_path(std::pair<int, int> unit) {
        return get_unit_folder_path(unit.first, unit.second);
    }
    nlohmann::json createUnitJson(std::pair<int, int> unit) {
        return createUnitJson(unit.first, unit.second);
    }
    sf::Texture createSlotTexture(std::pair<int, int> unit) {
        return createSlotTexture(unit.first, unit.second);
    }

};

static void modifyInt(int& tar, char op, float value) {
    switch (op) {
    case '=': tar = static_cast<int>(value); break;
    case '+': tar += static_cast<int>(value); break;
    case '*': tar = static_cast<int>((float)tar * value); break;
    default: tar = static_cast<int>(value); break;
    }
}
static void modifyFloat(float& target, char op, float value) {
    switch (op) {
    case '=': target = value; break;
    case '+': target += value; break;
    case '*': target *= value; break;
    default: target = value; break;
    }
}
static void modifyBitmask(int& target, char op, int value) {
    switch (op) {
    case '=': target = value; break;
    case '|': target |= value; break;
    case '&': target &= value; break;
    case '^': target ^= value; break;
    case '~': target &= ~value; break;
    default: target = value; break;
    }
}
static void modifyBitmask(size_t& target, char op, size_t value) {
    switch (op) {
    case '=': target = value; break;
    case '|': target |= value; break;
    case '&': target &= value; break;
    case '^': target ^= value; break;
    case '~': target &= ~value; break;
    default: target = value; break;
    }
}

void print_test(UnitStats* stats) {
    if (stats->unitId == 0 && stats->totalHits >= 3)
        std::cout << "hp: " << stats->maxHp << " - dmg: "
        << stats->hits[2].dmg << " - speed: " << stats->speed << std::endl;
}

void UnitStats::setup(const nlohmann::json& file) {
    std::pair<int, int> baseRange = { 0,0 };

    rechargeTime = file["stats"].value("recharge_timer", 0.f);
    unitId = file["unit_id"];
    team = file["team"];
    parts = get_parts_value(file);

    if (file.contains("hurtbox"))
        hurtBox = { file["hurtbox"]["width"], file["hurtbox"]["height"] };

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
    singleTarget = file["stats"]["single_target"];
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

UnitStats UnitStats::enemy(const nlohmann::json& file, float magnification) {
    UnitStats stats; 

    stats.setup(file);
    stats.maxHp = static_cast<int>(magnification * (float)stats.maxHp);
    for (int i = 0; i < stats.totalHits; i++)
        stats.hits[i].dmg = static_cast<int>(magnification * (float)stats.hits[i].dmg);

    return stats;
}
UnitStats UnitStats::player(const nlohmann::json& file, int core) {
    UnitStats stats;
    stats.setup(file);

    print_test(&stats);

    int gear = file.value("gear", 1);
    if (file.contains("cores") && core >= 0)
        for (const std::string& coreStr : file["cores"][core])
            stats.apply_core_modifier(coreStr, gear);

    print_test(&stats);

    return stats;
}
UnitStats UnitStats::create_cannon(const nlohmann::json& baseFile, float magnification) {
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

const std::unordered_map<std::string, std::function<void(UnitStats&, char, const std::string&)>>
UnitStats::statModifiers = {
    {"maxHp", [](UnitStats& s, char op, const std::string& val) {
        modifyInt(s.maxHp, op, std::stof(val));
    }},
    {"speed", [](UnitStats& s, char op, const std::string& val) {
        modifyFloat(s.speed, op, std::stof(val));
    }},
    {"knockbacks", [](UnitStats& s, char op, const std::string& val) {
        modifyInt(s.knockbacks, op, std::stof(val));
    }},
    {"attackTime", [](UnitStats& s, char op, const std::string& val) {
        modifyFloat(s.attackTime, op, std::stof(val));
    }},
    {"sightRange", [](UnitStats& s, char op, const std::string& val) {
        modifyFloat(s.sightRange, op, std::stof(val));
    }},
    {"singleTarget", [](UnitStats& s, char op, const std::string& val) {
        s.singleTarget = (val == "true" || val == "1" || op == '+');
    }},
    {"unitTypes", [](UnitStats& s, char op, const std::string& val) {
        int bits = UnitStats::convert_string_to_type(val);
        modifyBitmask(s.unitTypes, op, bits);
    }},
    {"targetTypes", [](UnitStats& s, char op, const std::string& val) {
        int bits = UnitStats::convert_string_to_type(val);
        modifyBitmask(s.targetTypes, op, bits);
    }},
    {"immunities", [](UnitStats& s, char op, const std::string& val) {
        size_t bits = Augment::string_to_augment_type(val);
        modifyBitmask(s.immunities, op, bits);
    }},
    {"quickAugMask", [](UnitStats& s, char op, const std::string& val) {
        size_t bits = Augment::string_to_augment_type(val);
        modifyBitmask(s.quickAugMask, op, bits);
    }},
};

void UnitStats::apply_core_modifier(const std::string& coreStr, int gear) {
    size_t firstUnderscore = coreStr.find('_');
    size_t lastUnderscore = coreStr.rfind('_');

    std::string stat = coreStr.substr(0, firstUnderscore);
    char op = coreStr[firstUnderscore + 1];
    std::string value = coreStr.substr(lastUnderscore + 1);

    auto it = statModifiers.find(stat);
    if (it != statModifiers.end()) {
        it->second(*this, op, value);
    }
    else if (stat.starts_with("augment")) {
        AugmentType augType = Augment::string_to_augment_type(value);
        switch (op) {
        case '-': removeAugment(augType); break;
        case '+': addCoreAugment(UnitData::createUnitJson(unitId, gear), value); break;
        default:
            std::cout << "no operation found in this core: " << op << std::endl;
            removeAugment(augType);
        }
    }
    else { 
        // if its not "augment" or in the map, its hitx where x is the hitIndex
        int hitIndex = stat[3] - '0';
        modifyDmg(hitIndex, op, std::stof(value));
    }
}
void UnitStats::removeAugment(AugmentType augType) {
    augments.erase(std::remove_if(augments.begin(), augments.end(),
        [augType](const Augment& aug) {
            return aug.augType == augType;
        }));
}
void UnitStats::addCoreAugment(const nlohmann::json& file, const std::string& augStr) {
    if (!file.contains("core_augments")) {
        std::cout << "Unit [" << file["name"] << 
            "] does not have Core augment of type[" << augStr << "]" << std::endl;
        return;
    }

    AugmentType coreAugType = Augment::string_to_augment_type(augStr);
    if (coreAugType == NONE) {
        std::cout << "coreAugType was invalid" << std::endl;
        return;
    }

    for (auto& coreAug : file["core_augments"]) {
        AugmentType augType = Augment::string_to_augment_type(coreAug["augment"]);
        if (augType == NONE || augType != coreAugType) continue;    
        quickAugMask |= augType;
        augments.emplace_back(Augment::from_json(augType, coreAug));
    }
}
void UnitStats::modifyDmg(int hitIndex, char op, float value) {
    if (hits.size() <= hitIndex) {
        std::cout << "Run into error with Core Modifier (DMG) - total hits is <= hitIndex. Total Hits: "
            << hits.size() << " - hitIndex: " << hitIndex << std::endl;
    }

    int& dmg = hits[hitIndex].dmg;
    switch (op) {
    case '=': dmg = static_cast<int>(value); break;
    case '+': dmg += static_cast<int>(value); break;
    case '*': dmg = (int)(static_cast<float>(dmg) * value); break;
    default: dmg = static_cast<int>(value); break;
    }
}

Augment UnitStats::get_augment(AugmentType aug) const {
    for (auto& augment : augments)
        if (augment.augType == aug) return augment;

    return {};
}
int UnitStats::get_parts_value(const nlohmann::json& json) const {
    int p = json["stats"].value("parts_dropped", 0);
    p = json["stats"].value("parts_cost", p);
    p = json["stats"].value("parts", p);
    return p;
}

bool UnitStats::try_proc_augment(AugmentType targetAugment, int hit) const {
    for (auto& augment : augments) {
        if (augment.augType != targetAugment) continue;
        return augment.can_hit(hit) && Random::chance(augment.percentage);
    }

    return false;
}