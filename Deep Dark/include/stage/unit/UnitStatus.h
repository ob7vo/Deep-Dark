#pragma once
#include "StatusEffect.h"
#include "AugmentTypes.h"

class Unit;
struct UnitStats;
struct Augment;
struct Surge;

struct UnitStatus {
    int hp = 0;
    int shieldHp = 0;
    int kbIndex = 0;

    std::vector<StatusEffect> activeStatuses;
    AugmentType statusFlags;

    explicit UnitStatus(const UnitStats* stats);
    ~UnitStatus() = default;

    float calculate_damage_reduction(const std::vector<Augment>& augments) const;
    int calculate_damage_and_effects(Unit& hitUnit, const Unit& attackingUnit);

    int apply_effects(const Unit& hitUnit, const std::vector<Augment>& augments, int hitIndex, int dmg = 0);
    void add_status_effect(const Augment& aug);
    void update_status_effects(Unit& unit, float deltaTime);
    bool can_proc_status(const Unit& unit, const Augment& augment, int hitIndex = 0) const;

    bool damage_shield(int& dmg, const UnitStats* _stats = nullptr);
    bool take_damage(Unit& hitUnit, const Unit& attackingUnit);
    bool take_damage(Unit& hitUnit, const Surge& surge);
    bool take_damage(Unit& hitUnit, int dmg, bool shove = false);

    bool met_knockback_threshold(int oldHp, const UnitStats* stats);
    bool try_proc_survive(const UnitStats* dyingUnitStats);

    inline bool can_phase() const { return has(statusFlags & AugmentType::PHASE); }
    inline bool can_transform() const { return has(statusFlags & AugmentType::TRANSFORM); }

    // Health
    inline bool has_shield_up() const { return shieldHp > 0; }
    inline bool dead() const { return hp <= 0; }

    // Status Conditions
    inline bool slowed() const { return has(statusFlags & AugmentType::SLOW); }
    inline bool overloaded() const { return has(statusFlags & AugmentType::OVERLOAD); }
    inline bool weakened() const { return has(statusFlags & AugmentType::WEAKEN); }
    inline bool blinded() const { return has(statusFlags & AugmentType::BLIND); }
    inline bool corroded() const { return has(statusFlags & AugmentType::CORRODE); }
    inline bool short_circuited() const { return has(statusFlags & AugmentType::SHORT_CIRCUIT); }
    inline bool infected() const { return has(statusFlags & AugmentType::VIRUS); }
};