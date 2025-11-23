#pragma once
#include "StatusEffect.h"
#include "UnitData.h"

class Unit;
struct UnitStats;
struct Surge;

struct UnitStatus {
    int hp = 0;
    int shieldHp = 0;
    int kbIndex = 0;

    std::vector<StatusEffect> activeStatuses;
    size_t statusFlags;

    explicit UnitStatus(const UnitStats* stats);

    float calculate_damage_reduction(const std::vector<Augment>& augments) const;
    int calculate_damage_and_effects(Unit& hitUnit, Unit& attackingUnit);

    int apply_effects(Unit& hitUnit, const std::vector<Augment>& augments, int hitIndex, int dmg = 0);
    void add_status_effect(const Augment& aug);
    void update_status_effects(Unit& unit, float deltaTime);
    bool can_proc_status(Unit& unit, Augment augment, int hitIndex = 0) const;

    bool damage_shield(int& dmg, const UnitStats* _stats = nullptr);
    bool take_damage(Unit& hitUnit, Unit& attackingUnit);
    bool take_damage(Unit& hitUnit, Surge& surge);
    bool take_damage(Unit& hitUnit, int dmg, bool shove = false);

    bool met_knockback_threshold(int oldHp, const UnitStats* stats);
    bool try_proc_survive(const UnitStats* dyingUnitStats);
    inline bool can_phase() const { return statusFlags & PHASE; }

    // Health
    inline bool has_shield_up() const { return shieldHp > 0; }
    inline bool dead() const { return hp <= 0; }

    // Status Conditions
    inline bool slowed() const { return statusFlags & AugmentType::SLOW; }
    inline bool overloaded() const { return statusFlags & AugmentType::OVERLOAD; }
    inline bool weakened() const { return statusFlags & AugmentType::WEAKEN; }
    inline bool blinded() const { return statusFlags & AugmentType::BLIND; }
    inline bool corroded() const { return statusFlags & AugmentType::CORRODE; }
    inline bool short_circuited() const { return statusFlags & AugmentType::SHORT_CIRCUIT; }
    inline bool infected() const { return statusFlags & AugmentType::VIRUS; }
};