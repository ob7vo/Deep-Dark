#include "UnitData.h"
#include <algorithm>

namespace UnitData {
    std::string get_unit_folder_path(int id, int gear) {
        return gear <= 1 ? std::format("configs/unit_data/{}/", id) :
            std::format("configs/unit_data/{}/gear{}/", id, gear);
    }
    nlohmann::json get_unit_json(int id, int gear) {
        const std::string path = get_unit_folder_path(id, gear) + "unit_data.json";
        //std::cout << path << std::endl;
        std::ifstream file(path);

        try {
            return nlohmann::json::parse(file);
        }
        catch (const nlohmann::json::parse_error& e) {
            std::cerr << "JSON parse error in " << path << ": " << e.what() << std::endl;
            return nlohmann::json();
        }
    }
    sf::Texture get_slot_texture(int id, int gear) {
        const std::string path = id >= 0 ? get_unit_folder_path(id, gear) + "slot.png" :
            "sprites/defaults/empty_slot.png";

        sf::Texture slotTex;

        if (!slotTex.loadFromFile(path)) {
            std::cerr << "wrong slot texture path: [" << path << 
                "], loading default instead" << std::endl;
            (void)slotTex.loadFromFile("sprites/defaults/empty_slot.png");
        }

        return slotTex;
    }

    std::string get_unit_folder_path(std::pair<int, int> unit) {
        return get_unit_folder_path(unit.first, unit.second);
    }
    nlohmann::json get_unit_json(std::pair<int, int> unit) {
        return get_unit_json(unit.first, unit.second);
    }
    sf::Texture get_slot_texture(std::pair<int, int> unit) {
        return get_slot_texture(unit.first, unit.second);
    }

};

static void modifyInt(int& tar, char op, float value) {
    switch (op) {
    case '=': tar = (int)value; break;
    case '+': tar += (int)value; break;
    case '*': tar = (int)(tar * value); break;
    }
}
static void modifyFloat(float& target, char op, float value) {
    switch (op) {
    case '=': target = value; break;
    case '+': target += value; break;
    case '*': target *= value; break;
    }
}
static void modifyBitmask(int& target, char op, int value) {
    switch (op) {
    case '=': target = value; break;
    case '|': target |= value; break;
    case '&': target &= value; break;
    case '^': target ^= value; break;
    case '~': target &= ~value; break;
    }
}
static void modifyBitmask(size_t& target, char op, size_t value) {
    switch (op) {
    case '=': target = value; break;
    case '|': target |= value; break;
    case '&': target &= value; break;
    case '^': target ^= value; break;
    case '~': target &= ~value; break;
    }
}

void print_test(UnitStats* stats) {
    if (stats->unitId == 0 && stats->totalHits >= 3)
        std::cout << "hp: " << stats->maxHp << " - dmg: "
        << stats->hits[2].dmg << " - speed: " << stats->speed << std::endl;
}
UnitStats UnitStats::enemy(const nlohmann::json& file, float magnification) {
    UnitStats stats; 

    stats.setup(file);
    stats.maxHp = (int)(stats.maxHp * magnification);
    for (int i = 0; i < stats.totalHits; i++)
        stats.hits[i].dmg = (int)(stats.hits[i].dmg * magnification);

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
const std::unordered_map<std::string, std::function<void(UnitStats&, char, const std::string&)>>
UnitStats::statModifiers = {
    {"maxHp", [](UnitStats& s, char op, const std::string& val) {
        modifyInt(s.maxHp, op, std::stof(val));
    }},
    {"speed", [](UnitStats& s, char op, const std::string& val) {
        modifyFloat(s.speed, op, std::stof(val));
    }},
    {"knockbacks", [](UnitStats& s, char op, const std::string& val) {
        modifyInt(s.knockbacks, op, std::stoi(val));
    }},
    {"attackTime", [](UnitStats& s, char op, const std::string& val) {
        modifyFloat(s.attackTime, op, std::stof(val));
    }},
    {"sightRange", [](UnitStats& s, char op, const std::string& val) {
        modifyFloat(s.sightRange, op, std::stof(val));
    }},
    {"singleTarget", [](UnitStats& s, char op, const std::string& val) {
        s.singleTarget = (val == "true" || val == "1");
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

void UnitStats::apply_core_modifier(const std::string coreStr, int gear) {
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
        case '+': addCoreAugment(UnitData::get_unit_json(unitId, gear), value); break;
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

void UnitStats::addCoreAugment(const nlohmann::json& file, std::string augStr) {
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
    case '=': dmg = (int)value; break;
    case '+': dmg += (int)value; break;
    case '*': dmg = (int)(dmg * value); break;
    }
}