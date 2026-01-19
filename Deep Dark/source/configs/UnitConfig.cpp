#include "pch.h"
#include "UnitConfig.h"

namespace UnitConfig {
    std::string getUnitFolderPath(int id) {
        return std::format("configs/unit_data/{}/", id);
    }
    std::string getUnitGearPath(int id, int gear, bool throwError) {
        if ((gear < 1 || gear > 3) && throwError)
            throw InvalidGearError(gear, id);

        return std::format("configs/unit_data/{}/gear{}/", id, gear);
    }
    nlohmann::json createUnitJson(int id, int gear)  {
        const std::string path = getUnitGearPath(id, gear) + "unit_data.json";
        std::ifstream file(path);

        try {
            return nlohmann::json::parse(file);
        }
        catch (const nlohmann::json::parse_error& e) {
            std::cerr << "JSON parse error in " << path << ": " << e.what() << std::endl;
            return nlohmann::json();
        }
    }
    nlohmann::json createSummonJson(int id) {
        const std::string path = getUnitFolderPath(id) + "summon/unit_data.json";
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
        const std::string path = id >= 0 ? getUnitGearPath(id, gear) + "slot.png" :
            "sprites/defaults/empty_slot.png";

        sf::Texture slotTex;

        if (!slotTex.loadFromFile(path)) {
            (void)slotTex.loadFromFile("sprites/defaults/empty_slot.png");
        }

        return slotTex;
    }
    int getMaxGear(int id) {
        int gears = 1;
        while (std::filesystem::exists(getUnitGearPath(id, gears + 1, false)))
            gears++;

        return gears;
    }

    // Overloadss
    std::string getUnitGearPath(std::pair<int, int> unit) {
        return getUnitGearPath(unit.first, unit.second);
    }
    nlohmann::json createUnitJson(std::pair<int, int> unit) {
        return createUnitJson(unit.first, unit.second);
    }
    sf::Texture createSlotTexture(std::pair<int, int> unit) {
        return createSlotTexture(unit.first, unit.second);
    }

    bool shouldFlipSprite(int id) {
        switch (id) {
        case 104: return true;
        default: return false;
        }

        return false;
    }
    sf::Color getGearColor(int id, int gear) {
        switch (id) {
        case 103: {
            switch (gear) {
            case 1: return sf::Color::White;
            case 2: return sf::Color::Red; // find the brown rgb
            case 3: return sf::Color::Blue;
            default: throw InvalidGearError(gear, id);
            }
            break;
        }
        default: return sf::Color::White;
        }

        return sf::Color::White;
    }
};
