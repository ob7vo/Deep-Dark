#pragma once
#include "AugmentTypes.h"
#include "Observer.h"
#include <vector>

struct UnitPool;
class Unit;

class UnitAbilityObserver : public IObserver {
private:
    struct UnitAugmentTimer {
        AugmentType augmentType;
        int unitSpawnIndex;
        int unitPoolIndex;
        float cooldown;
        int hpAtStart;
    };

    std::vector<UnitAugmentTimer> timers = {};
    UnitPool* unitPool = nullptr;

public:
    explicit UnitAbilityObserver(UnitPool* pool) : unitPool(pool) {}

    // Register a unit to be tracked
    void registerUnit(AugmentType augmentType, int unitSpawnIndex, int unitPoolIndex, 
        float triggerInterval, int extraVal = 0) 
    {
        timers.emplace_back(augmentType, unitSpawnIndex,  unitPoolIndex,
            triggerInterval, extraVal);
    }

    // Unregister when unit dies/removed
    void unregisterUnit(int unitPoolIndex) {
        timers.erase(
            std::remove_if(timers.begin(), timers.end(),
                [unitPoolIndex](const UnitAugmentTimer& t)
                { return t.unitPoolIndex == unitPoolIndex; }),
            timers.end()
        );
    }

    bool notify(const Event& event) override;

private:
    bool triggerAbility(Unit& unit, UnitAugmentTimer& timer) const;
    inline void removeTimer(int i) {
        timers[i] = timers.back();
        timers.pop_back();
    }
};