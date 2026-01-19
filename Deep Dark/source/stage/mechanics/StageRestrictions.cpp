#include "pch.h"
#include "StageRestrictions.h"

void UnitRestrictions::reset() {
    for (auto& bits : restrictedGears) bits.reset();
    maxUsableSlots = 10;
    descriptions.clear();
}

bool UnitRestrictions::is_allowed(int unitId, int gear) const {
    return !restrictedGears[unitId][gear];
}

void RestrictionParser::apply_unit_restrictions(UnitRestrictions& restrictions,
    const nlohmann::json& stageSetJson) {
    restrictions.reset();

    if (!stageSetJson.contains("restrictions")) return;

    for (const auto& restriction : stageSetJson["restrictions"]) {
        std::string type = restriction["type"];

        if (type == "only_gear")
            apply_gear_restriction(restrictions, restriction);
        else if (type == "max_slots")
            apply_slot_restriction(restrictions, restriction);
        else if (type == "cost_limit")
            apply_cost_restriction(restrictions, restriction);
        else if (type == "trait_only")
            apply_unit_type_restriction(restrictions, restriction);
        else if (type == "fixed_lineup")
            apply_fixed_lineup_restriction(restrictions, restriction);
    }
}

void RestrictionParser::apply_gear_restriction(UnitRestrictions& restrictions,
    const nlohmann::json& data) {
    int allowedGear = data["gear"];  // e.g., 1, 2, or 3
    std::string desc = std::format("Only Gear {} forms are allowed", allowedGear);
    restrictions.descriptions.push_back(desc);

    // Restrict all other gears
    for (int unitId = 0; unitId < UnitConfig::TOTAL_PLAYER_UNITS; unitId++)
        for (int gear = 0; gear < 3; gear++) 
            if (gear != allowedGear - 1)  // gear is 0-indexed
                restrictions.restrictedGears[unitId][gear] = true;
}
void RestrictionParser::apply_slot_restriction(UnitRestrictions& restrictions,
    const nlohmann::json& data) {
    restrictions.maxUsableSlots = data["max_slots"];
    restrictions.descriptions.push_back(
        std::format("Use no more than {} units", restrictions.maxUsableSlots)
    );
}
void RestrictionParser::apply_cost_restriction(UnitRestrictions& restrictions,
    const nlohmann::json& data) {
    std::string limitType = data["over_or_under"].get<std::string>();
    int partsLimit = data["parts_limit"];
    restrictions.descriptions.push_back(
        std::format("No units {} {} parts", limitType, partsLimit)
    );

    // Restrict units based on cost
    for (int unitId = 0; unitId < UnitConfig::TOTAL_PLAYER_UNITS; unitId++) {
        for (int gear = 0; gear < 3; gear++) {
            int cost = UnitConfig::createUnitJson(unitId, gear)["stats"]["parts_cost"];  // Your function
            if (limitType[0] == 'a') // above
                restrictions.restrictedGears[unitId][gear] = cost >= partsLimit;
            else if (limitType[0] == 'u') // under
                restrictions.restrictedGears[unitId][gear] = cost <= partsLimit;
            else 
                assert(false && "Incorrect parts limit type");
        }
    }
}
void RestrictionParser::apply_unit_type_restriction(UnitRestrictions& restrictions,
    const nlohmann::json& data) {
    std::unordered_set<std::string> requiredTypesSet(
        data["types"].begin(),
        data["types"].end()
    );
    //restrictions.descriptions.push_back(
   //     std::format("Only {} units allowed", requiredTypes)
  //  );

    // Units wont change traits naturally (without cores), so use base gear
    for (int unitId = 0; unitId < UnitConfig::TOTAL_PLAYER_UNITS; unitId++) {
        std::vector<std::string> unitTypes = UnitConfig::createUnitJson(unitId, 1)["types"];

        // Check if ANY unit type is in required set - O(1) per check
        bool hasMatchingType = std::ranges::any_of(unitTypes, [&](const auto& type) {
            return requiredTypesSet.contains(type);
            });

        if (!hasMatchingType) 
            restrictions.restrictedGears[unitId].set();  // Restrict all gears
    }
}
void RestrictionParser::apply_fixed_lineup_restriction(UnitRestrictions& restrictions,
    const nlohmann::json& data) {
    restrictions.descriptions.push_back("Use premade Lineup");
    
    // Set all gears as unusable
    for (auto& gears : restrictions.restrictedGears) gears.set();

    std::vector<std::pair<int, int>> lineup = data["lineup"];
    for (size_t i = 0; i < lineup.size(); i++)
        restrictions.restrictedGears[lineup[i].first][lineup[i].second] = false;
}