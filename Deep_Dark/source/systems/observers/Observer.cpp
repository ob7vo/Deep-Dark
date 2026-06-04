#include "pch.h"
#include "UnitPool.h"
#include "Unit.h"
#include "observers/Observer.h"
#include "observers/UnitAbilityObserver.h"

TimedEvent TimedEvent::oneSecond = TimedEvent(1.0f);

bool UnitAbilityObserver::notify(const Event& event)  {
    const auto& timedEvent = static_cast<const TimedEvent&>(event);
  
    for (size_t i = timers.size(); i--;) {
        auto& timer = timers[i];
        timer.cooldown -= timedEvent.timePassed;

        if (timer.cooldown <= 0) continue;

        // This confirms that the Unit is still alive
        if (auto* unit = unitPool->findUnitByIndex(timer.unitPoolIndex, timer.unitSpawnIndex)) {
            bool shouldRemove = triggerAbility(*unit, timer);

            if (shouldRemove) removeTimer(i);
        }
        else 
            removeTimer(i);
        
    }

    return true;
}
bool UnitAbilityObserver::triggerAbility(Unit& unit, UnitAugmentTimer& timer) const {
    if (const auto* discharge = unit.stats->get_augment(AugmentType::DISCHARGE)){
        unit.combat.discharge(timer.hpAtStart);

        timer.cooldown = discharge->data.onTimer.interval;
        timer.hpAtStart = unit.status.hp;

        return false;
    }
    if (const auto* overclock = unit.stats->get_augment(AugmentType::OVERCLOCK)){
        unit.status.process_new_status_effect(*overclock);
    }

    // Augment Timer will be REMOVED
    return true;
}