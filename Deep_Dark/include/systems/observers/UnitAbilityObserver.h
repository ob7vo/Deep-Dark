#pragma once
#include "AugmentTypes.h"
#include "UnitEnums.h"
#include "Observer.h"
#include "UnitInstanceIDs.h"
#include "Animation.h"
#include <vector>

struct Stage;
struct UnitPool;
class Unit;
struct SharedUnitData;

// This class is responsible for tracking Units with abilities that trigger on certain conditions and triggers them
// For example, abilities that are on Timers, or SALAGE. The timer is updated every singular second
class UnitAbilityObserver : public IObserver {
private:
    struct UnitAugmentTimer {
        AugmentType augmentType;
        int unitSpawnID;
        int unitPoolIndex;
        float cooldown;
        int hpAtStart;
    };
	struct RecentlyDeceasedUnit {
        const UnitStats* stats;
        UnitAnimMap* anims;
        int laneID;
		sf::Vector2f deathPosition;
        int team;
		float timeUntilRemoval = 3.f; 
	};  

    std::vector<UnitAugmentTimer> timers = {};
	std::vector<RecentlyDeceasedUnit> recentlyDeceasedUnits = {};
    std::vector<UnitInstanceIDs> unitsWithSalvage = {};

    UnitPool* unitPool = nullptr;
    std::vector<Lane>* lanes = nullptr;

public:
    explicit UnitAbilityObserver(UnitPool* pool, std::vector<Lane>* lanes = nullptr) : unitPool(pool), lanes(lanes) {}


    // Register a unit to be tracked    
    void registerTimer(AugmentType augmentType, int unitSpawnID, int unitPoolIndex, 
        float triggerInterval, int extraVal = 0) 
    {
        timers.emplace_back(augmentType, unitSpawnID,  unitPoolIndex,
            triggerInterval, extraVal);
    }
	void registerSalvage(size_t poolIndex, int unitSpawnID) {
       unitsWithSalvage.push_back({poolIndex, unitSpawnID });
	}
    void registerDeadUnit(const Unit& unit);

    // Unregister when unit dies/removed
    void unregisterUnit(int unitPoolIndex) {
        timers.erase(
            std::remove_if(timers.begin(), timers.end(),
                [unitPoolIndex](const UnitAugmentTimer& t)
                { return t.unitPoolIndex == unitPoolIndex; }),
            timers.end()
        );
        unitsWithSalvage.erase(
    std::remove_if(unitsWithSalvage.begin(), unitsWithSalvage.end(),
        [unitPoolIndex](const UnitInstanceIDs& id)
        { return id.poolIndex == unitPoolIndex; }), // adjust member name below
    unitsWithSalvage.end()
)       ; 
    }

    bool notify(const Event* event) override;
    void tick(float timePassed);

    inline bool has_salvage_unit() const { return !unitsWithSalvage.empty(); }
private:
    bool triggerAbility(Unit& unit, UnitAugmentTimer& timer) const;
};