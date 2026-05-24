#pragma once
#include <json_fwd.hpp>
#include <array>
#include <bitset>
#include "UnitConfig.h"

/// <summary>
/// Restrictions that actively inhibit the player during while playing a stage
/// </summary>
struct StageRestrictions {
    int maxUnits;

};
/// <summary>
/// Restrictions on what Units are allowed to be equipped before entering a stage
/// </summary>
struct UnitRestrictions {
    /// <summary> If true, that specific gear CANNOT be used </summary>
    std::bitset<UnitConfig::TOTAL_PLAYER_UNITS> restrictedUnits;
    int maxUsableSlots = 10;

    // Descriptions for UI
    std::vector<std::string> descriptions;

    // Clear all restrictions
    void reset();

    // Check if a specific unit form is allowed
    inline bool is_allowed(int unitID) const {
        return !restrictedUnits[unitID];
    }
};

class RestrictionParser {
public:
  //  static void apply_stage_restrictions(StageRestrictions& restrictions,
    //    const nlohmann::json& stageSetJson);
    static void apply_unit_restrictions(UnitRestrictions& restrictions,
        const nlohmann::json& stageSetJson);
private:
    /// <summary> Only X Units can be equipped </summary>
    static void apply_slot_restriction(UnitRestrictions& restrictions,
        const nlohmann::json& data);
    /// <summary> Only allow Units above/below costing X parts </summary>
    static void apply_cost_restriction(UnitRestrictions& restrictions,
        const nlohmann::json& data);
    /// <summary> Only allow Units that share a type X </summary>
    static void apply_unit_type_restriction(UnitRestrictions& restrictions,
        const nlohmann::json& data);
    // <summary> Slots X-Y must be a specific Unit and Gear </summary>
    static void apply_fixed_lineup_restriction(UnitRestrictions& restrictions,
        const nlohmann::json& data);
    // ... more restriction types
};