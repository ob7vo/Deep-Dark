#pragma once
#include "Easing.hpp"
#include "UnitEnums.h"

const EasingType noEase = EasingType::COUNT;
struct UnitTween {
    float duration = 0.f;
    float elapsedTime = 0.0f;
    bool isComplete = false;
    EasingType easingFuncX = EasingType::COUNT;
    EasingType easingFuncY = EasingType::COUNT;
    RequestType tweenType = RequestType::NONE;
    sf::Vector2f pos;
    sf::Vector2f startVec;
    sf::Vector2f endVec;

    UnitTween(sf::Vector2f val, const sf::Vector2f& end, float dur, RequestType type,
        EasingType funcX = EasingType::COUNT, EasingType funcY = EasingType::COUNT) :
        duration(dur), pos(val), startVec(val), endVec(end), tweenType(type),
        easingFuncX(funcX), easingFuncY(funcY) {}
    UnitTween() = default;
    ~UnitTween() = default;

    void update(float deltaTime) {
        if (isComplete) return;
        elapsedTime += deltaTime;

        if (elapsedTime >= duration) {
            elapsedTime = duration;
            updateValue();
            isComplete = true;
        }
        else updateValue();
    }
    float getEasedTX() const {
        float t = elapsedTime / duration;
        return easingFuncX < EasingType::COUNT ? 
            easeFuncArr[static_cast<int>(easingFuncX)](t) : t;
    }
    float getEasedTY() const {
        float t = elapsedTime / duration;
        return easeFuncArr[static_cast<int>(easingFuncY)](t);
    }
    inline sf::Vector2f update_and_get(float deltaTime) {
        update(deltaTime);
        return pos;
    }
    inline void updateValue() {
        if (easingFuncX < EasingType::COUNT) {
            float t = getEasedTX();
            pos.x = startVec.x + (endVec.x - startVec.x) * t;
        }
        if (easingFuncY < EasingType::COUNT) {
            float t = getEasedTY();
            pos.y = startVec.y + (endVec.y - startVec.y) * t;
        }
    }
};
struct BaseTween
{
    float duration;
    float elapsedTime;
    bool isComplete;
    std::function<float(float)> easingFunc;
    std::function<void()> onComplete;

    explicit BaseTween(float dur) :
        duration(dur), elapsedTime(0.0f), isComplete(false) {}
    BaseTween() = default;
    virtual ~BaseTween() = default; 
    virtual void update(float deltaTime) {
        if (isComplete) return;

        elapsedTime += deltaTime;

        if (elapsedTime >= duration) {
            elapsedTime = duration;
            updateValue();
            isComplete = true;
            if (onComplete) onComplete();
        } else updateValue();
    }
    virtual void updateValue() = 0;
    float getEasedT() const {
        float t = elapsedTime / duration;
        return easingFunc ? easingFunc(t) : t;
    }
    void stop() { isComplete = true; if (onComplete) onComplete(); } // stop tween and call onComplete
    void cancel() { isComplete = true; } // stop tweens and dont call onComplete
};
struct ChainableTween {
private:
    std::shared_ptr<BaseTween> tween;
public:
    ChainableTween() = default;
    explicit ChainableTween(std::shared_ptr<BaseTween> t) : tween(t) {}
    ~ChainableTween() = default; 

    ChainableTween& setEase(const std::function<float(float)>& ease) {
        if (tween) tween->easingFunc = ease;
        return *this;
    }
    ChainableTween& setOnComplete(const std::function<void()>& callback) {
        if (tween) tween->onComplete = callback;
        return *this;
    }
    std::shared_ptr<BaseTween> getTween() const { return tween; }
};
struct FloatTween : public BaseTween {
    float* target;
    float start;
    float end;

    FloatTween(float* tar, float end, float dur) :
        BaseTween(dur), target(tar), start(*tar), end(end) {}

    void updateValue() override {
        if (target) {
            float t = getEasedT();
            *target = start + (end - start) * t;
        }
        else isComplete = true;
    }
};
struct Vector2Tween : public BaseTween {
    sf::Vector2f* target;
    sf::Vector2f startVec;
    sf::Vector2f endVec;

    Vector2Tween(sf::Vector2f* tar, const sf::Vector2f& end, float dur) :
        BaseTween(dur), target(tar), startVec(*tar), endVec(end) {}

    inline void updateValue() override {
        if (target) {
            float t = getEasedT();
            *target = startVec + (endVec - startVec) * t;
        }
        else isComplete = true;
    }
};


class Tween {
private:
    static inline std::unordered_map<void*, std::shared_ptr<BaseTween>> activeTweens;

public:
    static ChainableTween move(sf::Vector2f* target, const sf::Vector2f& destination, float duration, bool overwrite = false) {
        if (isTweening(target) && !overwrite) return ChainableTween();
        auto tween = std::make_shared<Vector2Tween>(target, destination, duration);
        activeTweens[static_cast<void*>(target)] = tween;

        return ChainableTween(tween);
    }
    static ChainableTween move(float* target, float destination, float duration, bool overwrite = false) {
        if (isTweening(target) && !overwrite) return ChainableTween();
        auto tween = std::make_shared<FloatTween>(target, destination, duration);
        activeTweens[static_cast<void*>(target)] = tween;

        return ChainableTween(tween);
    }

    static inline void updateAll(float deltaTime) {
        std::erase_if(activeTweens, [](const auto& pair) {
            return pair.second->isComplete;
            });

        // Then update remaining tweens separately
        for (auto& [address, tween] : activeTweens) {
            tween->update(deltaTime);
        }
    }

    template<typename T>
    static inline bool isTweening(T* target) {
        return activeTweens.find(static_cast<void*>(target)) != activeTweens.end();
    }
    template<typename T>
    static inline void stop(T* target) {
        if (!isTweening(target)) return;
        activeTweens[static_cast<void*>(target)]->stop();
    }
    template<typename T>
    static inline void cancel (T* target) { 
        if (!isTweening(target)) return;
        
        auto key = static_cast<void*>(target);
        activeTweens[key]->cancel(); 
        activeTweens.erase(key);
    }

    static size_t getActiveCount() { return activeTweens.size(); }
};

