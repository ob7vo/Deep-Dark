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
    inline std::string wrapString(const std::string& str) {
        return std::format("[{}]", str);
    }
}
namespace FolderPaths {
    inline std::string path(std::string_view folder, std::string_view file) {
        return std::string(folder) + std::string(file);
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
}