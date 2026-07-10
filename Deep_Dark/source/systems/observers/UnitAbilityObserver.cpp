#include "pch.h"
#include "UnitPool.h"
#include "Stage.h"
#include "Unit.h"
#include "Animation.h"
#include "observers/Observer.h"
#include "observers/UnitAbilityObserver.h"
#include "Utils.h"

TimedEvent TimedEvent::oneSecond = TimedEvent(1.0f);

bool UnitAbilityObserver::notify(const Event* event) {
    // Usaully (I think always) one second
    if (auto timedEvent = dynamic_cast<const TimedEvent*>(event))
        tick(timedEvent->timePassed);


    return true;
}
void UnitAbilityObserver::tick(float timePassed) {
    for (size_t idx = timers.size(); idx--;) {
        auto& timer = timers[idx];
        timer.cooldown -= timePassed;

        if (timer.cooldown <= 0) continue;

        // This confirms that the Unit is still alive
        if (auto* unit = unitPool->findUnitByIndex(timer.unitPoolIndex, timer.unitSpawnID)) {
            bool shouldRemove = triggerAbility(*unit, timer);

            if (shouldRemove) Arrays::swap_n_pop(timers, idx);
        }
        else
            Arrays::swap_n_pop(timers, idx);

    }

    // Salvage
    for (size_t idx = unitsWithSalvage.size(); idx--;) {
        auto& salvager = unitsWithSalvage[idx];

        // Finds the NECROMANCER Unit, if they are still alive.
        // The NECROMANCER Unit should be alive, but run the check anyway
        if (auto* necromancer = unitPool->findUnitByIndex(salvager.poolIndex, salvager.spawnID)) {
            // If the Unit is not in its IDLE or MOVING animation, it can't necromance
            if (!necromancer->anim.can_enter_necromance_state()) continue;

            const Augment* salvage = necromancer->stats->get_augment(AugmentType::SALVAGE);
            int requiredKills = salvage->data.salvage.requiredKills;
            float range = salvage->data.salvage.reviveRange;
            bool spentRequiredKills = false;

            // Look for Units to revive
            for (size_t idx2 = recentlyDeceasedUnits.size(); idx2--;) {
                auto& deadUnit = recentlyDeceasedUnits[idx2];

                if (deadUnit.laneID != necromancer->get_lane() || deadUnit.stats->team != necromancer->stats->team
                    || !necromancer->enemy_in_range(deadUnit.deathPosition.x, -range, range))
                    continue;

                // NECROMANCE only requires spending kills once to revive all units within its radius
                if (!spentRequiredKills) {
                    if (necromancer->combat.kills < requiredKills) break;

                    // Mark the kills as spent so the rest of the units can be revived freely
                    necromancer->combat.kills -= requiredKills;
                    spentRequiredKills = true;
                }

                if (auto* revivedUnit = necromancer->stage->create_unit(deadUnit.laneID, deadUnit.stats, deadUnit.anims)) {
                    // Move the revived Unit into position and set its new hp
                    revivedUnit->movement.pos = deadUnit.deathPosition;
                    revivedUnit->status.hp = static_cast<int>(static_cast<float>(revivedUnit->stats->maxHp) * 
                        salvage->data.salvage.hpPercentageAfterRevival);

                    Arrays::swap_n_pop(recentlyDeceasedUnits, idx2);

                    // Start the animation for NECROMANCING 
                    if (!necromancer->anim.is_necromancing())
                        necromancer->anim.start(UnitAnimationState::NECROMANCING);
                }
            }
        }
        else // If the Unit for some reason isn't found and hasn't already been removed
            Arrays::swap_n_pop(unitsWithSalvage, idx);
    }
    for (size_t idx = recentlyDeceasedUnits.size(); idx--;) {
        auto& deadUnit = recentlyDeceasedUnits[idx];
        deadUnit.timeUntilRemoval -= timePassed;

        if (deadUnit.timeUntilRemoval <= 0)
            Arrays::swap_n_pop(recentlyDeceasedUnits, idx);
    }
}

void UnitAbilityObserver::registerDeadUnit(const Unit& unit) {
    recentlyDeceasedUnits.emplace_back(unit.stats, unit.anim.get_ani_map(), unit.movement.laneIdx, unit.movement.pos);
}

bool UnitAbilityObserver::triggerAbility(Unit& unit, UnitAugmentTimer& timer) const {
    if (const auto* discharge = unit.stats->get_augment(AugmentType::DISCHARGE)) {
        unit.combat.discharge(timer.hpAtStart);

        timer.cooldown = discharge->data.onTimer.interval;
        timer.hpAtStart = unit.status.hp;

        return false;
    }
    if (const auto* overclock = unit.stats->get_augment(AugmentType::OVERCLOCK)) {
        unit.status.process_new_status_effect(*overclock);
    }

    // Augment Timer will be REMOVED
    return true;
}