#include "pch.h"
#include "UnitConfig.h"
#include "UnitSaveData.h"

namespace UnitConfig {
    std::string getUnitFolderPath(int id) {
        return std::format("configs/unit_data/{}/", id);
    }
    nlohmann::json createUnitJson(int id)  {
        const std::string path = getUnitFolderPath(id) + "unit_data.json";
        std::ifstream file(path);

        try {
            return nlohmann::json::parse(file);
        }
        catch (const nlohmann::json::parse_error& e) {
            std::cerr << "JSON parse error in " << path << ": " << e.what() << std::endl;
            return nlohmann::json();
        }
    }
    // make summon folder 
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
    sf::Texture createSlotTexture(int id) {
        const std::string path = id >= 0 ? getUnitFolderPath(id) + "slot.png" :
            "sprites/defaults/empty_slot.png";

        sf::Texture slotTex;

        if (!slotTex.loadFromFile(path)) {
            (void)slotTex.loadFromFile("sprites/defaults/empty_slot.png");
        }

        return slotTex;
    }
    bool shouldFlipSprite(int id) {
        switch (id) {
        case 104: return true;
        default: return false;
        }

        return false;
    }
    sf::Color getUnitColor(int id) {
        /*
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
*/
        return sf::Color::White;
    }
};
