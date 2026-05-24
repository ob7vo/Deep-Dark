#include "pch.h"
#include "StageRestrictions.h"

void UnitRestrictions::reset() {
    restrictedUnits.reset();
    maxUsableSlots = 10;
    descriptions.clear();
}

void RestrictionParser::apply_unit_restrictions(UnitRestrictions& restrictions,
    const nlohmann::json& stageSetJson) {
    restrictions.reset();

    if (!stageSetJson.contains("restrictions")) return;

    for (const auto& restriction : stageSetJson["restrictions"]) {
        std::string type = restriction["type"];


        if (type == "max_slots")
            apply_slot_restriction(restrictions, restriction);
        else if (type == "cost_limit")
            apply_cost_restriction(restrictions, restriction);
        else if (type == "trait_only")
            apply_unit_type_restriction(restrictions, restriction);
        else if (type == "fixed_lineup")
            apply_fixed_lineup_restriction(restrictions, restriction);
    }
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
        int cost = UnitConfig::createUnitJson(unitId)["stats"]["parts_cost"];  // Your function
        if (limitType[0] == 'a') // above
            restrictions.restrictedUnits[unitId] = cost >= partsLimit;
        else if (limitType[0] == 'u') // under
            restrictions.restrictedUnits[unitId] = cost <= partsLimit;
        else 
            assert(false && "Incorrect parts limit type");
        
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
        std::vector<std::string> unitTypes = UnitConfig::createUnitJson(unitId)["types"];

        // Check if ANY unit type is in required set - O(1) per check
        bool hasMatchingType = std::ranges::any_of(unitTypes, [&](const auto& type) {
            return requiredTypesSet.contains(type);
            });

        if (!hasMatchingType) 
            restrictions.restrictedUnits[unitId] = true;  // Restrict all gears
    }
}
void RestrictionParser::apply_fixed_lineup_restriction(UnitRestrictions& restrictions,
    const nlohmann::json& data) {
    restrictions.descriptions.push_back("Use premade Lineup");
    
    // Set all gears as unusable
    restrictions.restrictedUnits.set();

    std::vector<int> lineup = data["lineup"].get<std::vector<int>>();
    for (size_t i = 0; i < lineup.size(); i++)
        restrictions.restrictedUnits[lineup[i]]= false;
}