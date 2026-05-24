#pragma once
#include "UnitMoveRequest.h"
#include <SFML/System/Vector2.hpp>

struct UnitTween {
    float duration = 0.f;
    float elapsedTime = 0.0f;
    bool active = false;

    UnitMoveRequestType tweenType = UnitMoveRequestType::NONE;
    sf::Vector2f startPos = { 0.f,0.f };
    sf::Vector2f endPos = { 0.f,0.f };

    UnitTween() = default;
    ~UnitTween() = default;

    void start(sf::Vector2f val, const sf::Vector2f& end, float dur, UnitMoveRequestType type = UnitMoveRequestType::PHASE);
    void update(sf::Vector2f& unitPos, float deltaTime);
    sf::Vector2f getEasedT() const;
    inline void updateValue(sf::Vector2f& unitPos) const {
        unitPos = startPos + ((endPos - startPos) * getEasedT());
    }
};