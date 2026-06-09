#pragma once
#include "UnitLaneTransferRequest.h"
#include <SFML/System/Vector2.hpp>

struct UnitTween {
    float duration = 0.f;
    float elapsedTime = 0.0f;
    bool active = false;

    UnitLaneTransferRequestType tweenType = UnitLaneTransferRequestType::NONE;
    sf::Vector2f startPos = { 0.f,0.f };
    sf::Vector2f endPos = { 0.f,0.f };

    UnitTween() = default;
    ~UnitTween() = default;

    void start(sf::Vector2f val, const sf::Vector2f& end, float dur, UnitLaneTransferRequestType type = UnitLaneTransferRequestType::PHASE);
    void update(sf::Vector2f& unitPos, float deltaTime);
    sf::Vector2f getEasedT() const;
    inline void updateValue(sf::Vector2f& unitPos) const {
        unitPos = startPos + ((endPos - startPos) * getEasedT());
    }
};