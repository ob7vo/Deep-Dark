#pragma once
#include <cmath>
#include <array>

const float M_PI = 3.1415927f;
namespace Easing {
    // Linear (no easing)
    inline float zero(float t) { return 0; }
    inline float linear(float t) { return t; }

    // ========== QUADRATIC ==========
    inline float easeInQuad(float t) {
        return t * t;
    }
    inline float easeOutQuad(float t) {
        return 1.0f - (1.0f - t) * (1.0f - t);
    }
    inline float easeInOutQuad(float t) {
        return t < 0.5f ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
    }
    // ========== CUBIC ==========
    inline float easeInCubic(float t) {
        return t * t * t;
    }
    inline float easeOutCubic(float t) {
        return 1.0f - std::pow(1.0f - t, 3.0f);
    }

    inline float easeInOutCubic(float t) {
        return t < 0.5f ? 4.0f * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
    }
    // ========== QUARTIC ==========
    inline float easeInQuart(float t) {
        return t * t * t * t;
    }
    inline float easeOutQuart(float t) {
        return 1.0f - std::pow(1.0f - t, 4.0f);
    }
    inline float easeInOutQuart(float t) {
        return t < 0.5f ? 8.0f * t * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 4.0f) / 2.0f;
    }
    // ========== SINE ==========
    inline float easeInSine(float t) {
        return 1.0f - std::cos((t * M_PI) / 2.0f);
    }
    inline float easeOutSine(float t) {
        return std::sin((t * M_PI) / 2.0f);
    }
    inline float easeInOutSine(float t) {
        return -(std::cos(M_PI * t) - 1.0f) / 2.0f;
    }
    // ========== EXPONENTIAL ==========
    inline float easeInExpo(float t) {
        return t == 0.0f ? 0.0f : std::pow(2.0f, 10.0f * (t - 1.0f));
    }
    inline float easeOutExpo(float t) {
        return t == 1.0f ? 1.0f : 1.0f - std::pow(2.0f, -10.0f * t);
    }
    inline float easeInOutExpo(float t) {
        return t == 0.0f ? 0.0f : t == 1.0f ? 1.0f : t < 0.5f
            ? std::pow(2.0f, 20.0f * t - 10.0f) / 2.0f
            : (2.0f - std::pow(2.0f, -20.0f * t + 10.0f)) / 2.0f;
    }
    // ========== CIRCULAR ==========
    inline float easeInCirc(float t) {
        return 1.0f - std::sqrt(1.0f - std::pow(t, 2.0f));
    }
    inline float easeOutCirc(float t) {
        return std::sqrt(1.0f - std::pow(t - 1.0f, 2.0f));
    }
    inline float easeInOutCirc(float t) {
        return t < 0.5f
            ? (1.0f - std::sqrt(1.0f - std::pow(2.0f * t, 2.0f))) / 2.0f
            : (std::sqrt(1.0f - std::pow(-2.0f * t + 2.0f, 2.0f)) + 1.0f) / 2.0f;
    }
    // ========== BACK (overshooting) ==========
    inline float easeInBack(float t) {
        const float c1 = 1.70158f;
        const float c3 = c1 + 1.0f;
        return c3 * t * t * t - c1 * t * t;
    }

    inline float easeOutBack(float t) {
        const float c1 = 1.70158f;
        const float c3 = c1 + 1.0f;
        return 1.0f + c3 * std::pow(t - 1.0f, 3.0f) + c1 * std::pow(t - 1.0f, 2.0f);
    }

    inline float easeInOutBack(float t) {
        const float c1 = 1.70158f;
        const float c2 = c1 * 1.525f;
        return t < 0.5f
            ? (std::pow(2.0f * t, 2.0f) * ((c2 + 1.0f) * 2.0f * t - c2)) / 2.0f
            : (std::pow(2.0f * t - 2.0f, 2.0f) * ((c2 + 1.0f) * (t * 2.0f - 2.0f) + c2) + 2.0f) / 2.0f;
    }

    // ========== ELASTIC (bouncy) ==========
    inline float easeInElastic(float t) {
        const float c4 = (2.0f * M_PI) / 3.0f;
        return t == 0.0f ? 0.0f : t == 1.0f ? 1.0f
            : -std::pow(2.0f, 10.0f * t - 10.0f) * std::sin((t * 10.0f - 10.75f) * c4);
    }
    inline float easeOutElastic(float t) {
        const float c4 = (2.0f * M_PI) / 3.0f;
        return t == 0.0f ? 0.0f : t == 1.0f ? 1.0f
            : std::pow(2.0f, -10.0f * t) * std::sin((t * 10.0f - 0.75f) * c4) + 1.0f;
    }
    inline float easeInOutElastic(float t) {
        const float c5 = (2.0f * M_PI) / 4.5f;
        return t == 0.0f ? 0.0f : t == 1.0f ? 1.0f : t < 0.5f
            ? -(std::pow(2.0f, 20.0f * t - 10.0f) * std::sin((20.0f * t - 11.125f) * c5)) / 2.0f
            : (std::pow(2.0f, -20.0f * t + 10.0f) * std::sin((20.0f * t - 11.125f) * c5)) / 2.0f + 1.0f;
    }

    // ========== BOUNCE ==========
    inline float easeOutBounce(float t) {
        const float n1 = 7.5625f;
        const float d1 = 2.75f;

        if (t < 1.0f / d1) {
            return n1 * t * t;
        }
        else if (t < 2.0f / d1) {
            return n1 * (t -= 1.5f / d1) * t + 0.75f;
        }
        else if (t < 2.5f / d1) {
            return n1 * (t -= 2.25f / d1) * t + 0.9375f;
        }
        else {
            return n1 * (t -= 2.625f / d1) * t + 0.984375f;
        }
    }
    inline float easeInBounce(float t) {
        return 1.0f - easeOutBounce(1.0f - t);
    }
    inline float easeInOutBounce(float t) {
        return t < 0.5f
            ? (1.0f - easeOutBounce(1.0f - 2.0f * t)) / 2.0f
            : (1.0f + easeOutBounce(2.0f * t - 1.0f)) / 2.0f;
    }
}
enum class EasingType {
    LINEAR,
    OUT_BOUNCE,
    OUT_CUBIC,
    IN_OUT_SINE,
    OUT_QUART,
    OUT_BACK,
    COUNT
};
using EasingFunc = float(*)(float);
static const auto easeFuncArr = std::array<EasingFunc, 7>{
    Easing::linear,
    Easing::easeOutBounce,
    Easing::easeOutCubic,
    Easing::easeInOutSine,
    Easing::easeOutQuart,
    Easing::easeOutBack,
    Easing::zero
};

const EasingType noEase = EasingType::COUNT;
const std::array<std::pair<EasingType, EasingType>, 8> unitTweenEasings = { {
    {EasingType::OUT_CUBIC, EasingType::COUNT}, // KNOCKBACK
    {EasingType::COUNT, EasingType::IN_OUT_SINE}, // FALL
    {EasingType::COUNT, EasingType::OUT_BOUNCE}, // SQUASH
    {EasingType::COUNT, EasingType::OUT_QUART}, // LAUNCH
    {EasingType::COUNT, EasingType::OUT_BOUNCE}, // DROP_FROM_LAUNCH
    {EasingType::LINEAR, EasingType::OUT_BACK}, // JUMP
    {EasingType::LINEAR, EasingType::COUNT}, // LEAP
    {EasingType::LINEAR, EasingType::COUNT}
} };
