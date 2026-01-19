#include "pch.h"
#include "SaveSystem.h"
#include "UnitConfig.h"
#include "StageConfig.h"

using json = nlohmann::json;

SaveSystem SaveSystem::s_instance;

void SaveSystem::initialize() {
    if (!saveFileExists("player_save.json"))
        create_player_save();
    if (!saveFileExists("stage_save.json"))
		create_stage_save();
	if (!saveFileExists("unit_save.json"))
		create_unit_save();
}

/* I made the save data for my unit sand stages an aray for each individualy Unit and Stage (with their indexes being their IDs)
    Is this fine to do it like this? And should the rest of save data be in one json now? Like currency, playtime, and other stats?
    I'm still not sure on how I should handle currency. Specifically on how to distrubute it to other parts of code (static class? get directly from SaveSystem static class/save files)
*/
void SaveSystem::create_player_save() {
    json save;

    save["scrap_parts"] = 0; // Currency for Levels. From clearing stages
    save["modules"] = 0; // CUrrency for True Forms and Cores. From Challenges

    save["playtime"] = 0.f;
    save["total_stages_cleared"] = 0;
    save["total_challenges_cleared"] = 0;

    std::ofstream file(SAVE_FOLDER_PATH + "player_save.json");
    file << save.dump(2);  // The '2' means indent with 2 spaces
    file.close();
}
void SaveSystem::create_unit_save() {
    json save;

    save["units"] = json::array();
    // each array will have a size of const UNITS
    json unitData;

    unitData["level"] = 1;
    unitData["unlocked_status"] = 0; // 0 = NOT unlocked, 1 = unlocked, 2 = has true form
    unitData["cores"] = json::array();

    for (int i = 1; i < UnitConfig::MAX_CORES; i++)
        unitData["cores"].push_back(false);
    
    // Push the default data for each player unit
    for (int i = 1; i < UnitConfig::TOTAL_PLAYER_UNITS; i++)
        save.push_back(unitData);

    // set the first Unit as unlocked
    save["units"][0]["unlocked_status"] = 1;

    std::ofstream file(SAVE_FOLDER_PATH + "unit_save.json");
    file << save.dump(2);  // The '2' means indent with 2 spaces
    file.close();
}
void SaveSystem::create_stage_save() {
    // Created the array of stages
    json save;
    save["stages"] = json::array(); 

    // Start creating the data that each stage will need saved
    json stageData;

    stageData["completions"] = 0;

    stageData["challenges_cleared"] = json::array();
    for (int i = 0; i < StageConfig::CHALLENGES_COUNT; i++)
        stageData["challenges_cleared"].push_back(false);

    // Push the default data for each stage
    for (int i = 0; i < StageConfig::TOTAL_STAGES; i++)
        save["stages"].push_back(stageData);

    std::ofstream file(SAVE_FOLDER_PATH + "stage_save.json");
    file << save.dump(2);  // The '2' means indent with 2 spaces
    file.close();
}

void SaveSystem::load_save(std::string_view file) {

}

bool SaveSystem::saveFileExists(std::string_view file) {
    return std::filesystem::exists(SAVE_FOLDER_PATH + (std::string)file);
}
