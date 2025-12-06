#pragma once
#include "Easing.hpp"
#include "UnitMoveRequest.h"
#include <SFML/System/Vector2.hpp>

const EasingType noEase = EasingType::COUNT;
const std::array<std::pair<EasingType, EasingType>, 8> unitTweenEasings = { {
    {EasingType::OUT_CUBIC, EasingType::COUNT}, // KNOCKBACK
    {EasingType::COUNT, EasingType::IN_OUT_SINE}, // FALL
    {EasingType::COUNT, EasingType::OUT_BOUNCE}, // SQUASH
    {EasingType::COUNT, EasingType::OUT_QUART}, // LAUNCH
    {EasingType::COUNT, EasingType::OUT_BOUNCE}, // DROP_FROM_LAUNCH
    {EasingType::LINEAR, EasingType::OUT_BACK}, // JUMP
    {EasingType::LINEAR, EasingType::COUNT} // LEAP
    // PHASE
    /*KNOCKBACK, tween.easingFuncX = EasingType::OUT_CUBIC;
    FALL,
    SQUASH,
    LAUNCH,
    DROP_FROM_LAUNCH,
    JUMP,
    LEAP,
    PHASE,
  *      TELEPORT,
**    */
} };

struct UnitTween {
    float duration = 0.f;
    float elapsedTime = 0.0f;
    bool active = false;

    RequestType tweenType = RequestType::NONE;
    sf::Vector2f startPos;
    sf::Vector2f endPos;

    UnitTween() = default;
    ~UnitTween() = default;

    void start(sf::Vector2f val, const sf::Vector2f& end, float dur, RequestType type = RequestType::PHASE);
    void update(sf::Vector2f& unitPos, float deltaTime);
    sf::Vector2f getEasedT() const;
    void updateValue(sf::Vector2f& unitPos) const;
    float getEasedTX(EasingType easeX) const {
        float t = elapsedTime / duration;
        return easeX < EasingType::COUNT ?
            easeFuncArr[static_cast<int>(easeX)](t) : t;
    }
    float getEasedTY(EasingType easeY) const {
        float t = elapsedTime / duration;
        return easeFuncArr[static_cast<int>(easeY)](t);
    }
};