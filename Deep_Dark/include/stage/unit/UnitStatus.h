#pragma once
#include <optional>
#include "StatusEffect.h"
#include "AugmentTypes.h"

class Unit;
struct UnitStats;
struct Augment;
struct Surge;

// Subclass that handles a Unit's HP and status effects. "ownerUnit" means that the unit is the own of this class
struct UnitStatus {
    Unit& owner;

    int hp = 0;
    int shieldHp = 0;
    int kbIndex = 1;

    std::vector<StatusEffect> activeStatuses = {};
    AugmentType statusFlags = AugmentType::NONE;

    explicit UnitStatus(Unit& ownerUnit) : owner(ownerUnit) {}
    ~UnitStatus() = default;

    void setup(const UnitStats* stats);

    float calculate_damage_reduction(const std::vector<Augment>& augments) const;
    float calculate_damage_boost(const std::vector<Augment>& augments) const;
    int calculate_damage_and_effects(const Unit& attackingUnit);

    /// <summary> Process an augment to turn it to a status affect </summary>
    /// <param name="fromLink"></param> Needed to avoid multiple linkers infintely repeating links
    void process_new_status_effect(const Augment& aug, bool linked = false);
    void add_status_effect(const StatusEffect& statusEffect);
    void apply_on_hit_status_effects(const std::vector<Augment>& augments, int hitIndex);
    void trigger_health_threshold_augments(); // called after taking damage
    void link_augment(const Augment& status);
    void update_status_effects(float deltaTime);
    bool can_proc_status(const Augment& augment, int hitIndex = 0) const;
    /// <summary> Gets the matching active status effect if there is one </summary>
    std::optional<StatusEffect> get_status_effect(AugmentType effType) const;

    bool damage_shield(int dmg, const UnitStats* _stats = nullptr);
    bool take_damage(const Unit& attackingUnit);
    bool take_damage(const Surge& surge);
    bool take_damage(int dmg, bool shove = false);

    bool met_knockback_threshold(int oldHp);
    bool try_proc_survive();
    void syphon(const Augment* syphon);

    inline bool can_phase() const { return has(statusFlags & AugmentType::PHASE); }
    inline bool can_transform() const { return has(statusFlags & AugmentType::TRANSFORM); }

    // Health
    inline bool has_shield_up() const { return shieldHp > 0; }
    inline bool dead() const { return hp <= 0; }

    // Status Conditions
    inline float get_overload_multiplier() const {
        const auto eff = get_status_effect(AugmentType::OVERLOAD);
        return eff ? eff->value : 1.0f;
    }
    inline float get_weaken_multiplier() const {
        const auto eff = get_status_effect(AugmentType::WEAKEN);
        return eff ? eff->value : 1.0f;
    }
    inline float get_corrosion_multiplier() const {
        const auto eff = get_status_effect(AugmentType::CORRODE);
        return eff ? eff->value : 1.0f;
    }
    inline float get_overclock_multiplier() const {
        const auto eff = get_status_effect(AugmentType::OVERCLOCK);
        return eff ? eff->value : 1.0f;
    }
    inline float get_reinforcement_multiplier() const {
        const auto eff = get_status_effect(AugmentType::REINFORCE);
        return eff ? eff->value : 1.0f;
    }
    inline float get_strengthen_multiplier() const {
        const auto eff = get_status_effect(AugmentType::STRENGTHEN);
        return eff ? eff->value : 1.0f;
    }
    inline float get_blindness_multiplier() const {
        const auto eff = get_status_effect(AugmentType::BLIND);
        return eff ? eff->value : 1.0f;
    }
    inline bool is_bolted() const { return has(statusFlags, AugmentType::BOLT); }
    inline bool is_slowed() const { return has(statusFlags, AugmentType::SLOW); }
    inline bool is_short_circuited() const { return has(statusFlags & AugmentType::SHORT_CIRCUIT); }
    inline bool is_infected() const { return has(statusFlags & AugmentType::VIRUS); }
    inline bool is_scoped() const { return has(statusFlags, AugmentType::SCOPE); }
};