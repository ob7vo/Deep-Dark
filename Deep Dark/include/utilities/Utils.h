#pragma once
#include <SFML/System/Vector2.hpp>
#include <random>
#include <format>

namespace Random {
    inline std::mt19937& generator() {
        static std::mt19937 gen(std::random_device{}());
        return gen;
    }

    inline int r_int(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(generator());
    }

    inline float r_float(float min, float max) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(generator());
    }

    inline bool chance(float probability) {
        return r_float(0.0f, 100.f) < probability;
    }
}
namespace Converting {
    template<typename T>
    inline float cast_f(T var) {
        return static_cast<float>(var);
    }
    template<typename T>
    inline int cast_i(T var) {
        return static_cast<int>(var);
    }
}
namespace Printing {
    // Wrap a string in square brakets []
    inline std::string wrap(std::string_view str) {
        return std::format("[{}]", str);
    }
    // Wrap a string in Parenthesis ()
    inline std::string wrapP(std::string_view str) {
        return std::format("({})", str);
    }
    template<typename T>
    inline std::string vec2(sf::Vector2<T> v2) {
        return std::format("({}, {})", v2.x, v2.y);
    }
}
namespace FolderPaths {
    inline std::string path(std::string_view folder, std::string_view file) {
        return std::format("{}{}", folder, file);
    }

    inline constexpr std::string_view effectsPath = "sprites/effects/";
    inline constexpr std::string_view workshopPath = "sprites/ui/workshop/";
    inline constexpr std::string_view armoryPath = "sprites/ui/armory_menu/";
    inline constexpr std::string_view stageSelectPath = "sprites/ui/stage_select/";
    inline constexpr std::string_view stageUIPath = "sprites/ui/stage_ui/";
    inline constexpr std::string_view startMenuPath = "sprites/ui/start_menu/";
    inline constexpr std::string_view iconsPath = "sprites/icons/";
    inline constexpr std::string_view defaultsPath = "sprites/defaults/";
    inline constexpr std::string_view uiPath = "sprites/ui/";
    inline constexpr std::string_view entitySpritesPath = "sprites/entities/";
    inline constexpr std::string_view baseSpritesPath = "sprites/entities/bases/";
    inline constexpr std::string_view surgeSpritesPath = "sprites/entities/surges/";
    inline constexpr std::string_view trapSpritesPath = "sprites/entities/traps/";


}
namespace Collision {
    inline bool AABB(sf::Vector2f posA, sf::Vector2f boxA, sf::Vector2f posB, sf::Vector2f boxB) {
        return (posA.x < posB.x + boxB.x && posA.x + boxA.x > posB.x &&
            posA.y < posB.y + boxB.y && posA.y + boxA.y > posB.y);
    }
    inline bool AABB(sf::FloatRect a, sf::FloatRect b) {
        return (a.position.x < b.position.x + b.size.x &&
            a.position.x + a.size.x > b.position.x &&
            a.position.y < b.position.y + b.size.y &&
            a.position.y + a.size.y > b.position.y);
    }
    inline bool range(float posA, float posB, float minRange, float maxRange) {
        float dist = std::abs(posA - posB);
        return dist >= minRange && dist <= maxRange;
    }
}