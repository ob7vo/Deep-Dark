#pragma once
#include <SFML/System/Vector2.hpp>
#include "Easing.hpp"
#include <iostream>
#include <vector>
#include <memory>
#include <functional>
using Vector2 = sf::Vector2f;


struct BaseTween
{
    float duration;
    float elapsedTime;
    bool isComplete;
    std::function<float(float)> easingFunc;
    std::function<void()> onComplete;

    BaseTween(float dur) :
        duration(dur), elapsedTime(0.0f), isComplete(false) {}
    virtual ~BaseTween() = default; // { printf("finished tween\n"); }
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
    ChainableTween(std::shared_ptr<BaseTween> t) : tween(t) {}
    ~ChainableTween() = default; // { printf("done chaining\n"); }

    ChainableTween& setEase(std::function<float(float)> ease) {
        if (tween) tween->easingFunc = ease;
        return *this;
    }
    ChainableTween& setOnComplete(std::function<void()> callback) {
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
    Vector2* target;
    Vector2 startVec;
    Vector2 endVec;

    Vector2Tween(Vector2* tar, const Vector2& end, float dur) :
        BaseTween(dur), target(tar), startVec(*tar), endVec(end) {}

    inline void updateValue() override {
        if (target) {
            float t = getEasedT();
            *target = startVec + (endVec - startVec) * t;
        }
        else isComplete = true;
    }
    //Vector2 customBounceEasing(float t){
    //    float x = getEasedT(); // Linear for X
    //    float y = Easing::easeOutBounce(t); // Bouncy for Y (or easeOutBounce, etc.)
    //    return { x, y };
    //};
};


class Tween {
private:
    static inline std::unordered_map<void*, std::shared_ptr<BaseTween>> activeTweens;

public:
    static ChainableTween move(Vector2* target, const Vector2& destination, float duration, bool overwrite = false) {
        if (isTweening(target) && !overwrite) return ChainableTween();
        std::shared_ptr<Vector2Tween> tween = std::make_shared<Vector2Tween>(target, destination, duration);
        activeTweens[static_cast<void*>(target)] = tween;
        //std::cout << "created vector tween" << std::endl;
        return ChainableTween(tween);
    }
    static ChainableTween move(float* target, float destination, float duration, bool overwrite = false) {
        if (isTweening(target) && !overwrite) return ChainableTween();
        std::shared_ptr<FloatTween> tween = std::make_shared<FloatTween>(target, destination, duration);
        activeTweens[static_cast<void*>(target)] = tween;
       // std::cout << "created float tween" << std::endl;
        return ChainableTween(tween);
    }

    static inline void updateAll(float deltaTime) {
        for (auto it = activeTweens.begin(); it != activeTweens.end();) {
            if (it->second->isComplete)
                it = activeTweens.erase(it);
            else {
                std::cout << "Updating tween at address: " << it->first << std::endl;
                it->second->update(deltaTime);  
                ++it;
            }
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
        if (!isTweening(target)) {
            //std::cout << "cant cancel non-tweening address: " << target << std::endl;
            return;
        }
        std::cout << "canceling tween of address: " << target << std::endl;
        void* key = static_cast<void*>(target);
        activeTweens[key]->cancel(); 
        activeTweens.erase(key);
    }

    static size_t getActiveCount() { return activeTweens.size(); }
};

