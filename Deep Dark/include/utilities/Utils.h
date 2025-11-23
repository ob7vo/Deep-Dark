#pragma once
#include <SFML/System/Vector2.hpp>
#include <random>

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