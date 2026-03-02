#include "pch.h"
#include "UnitSaveData.h"
#include "SaveSystem.h"
#include "UnitConfig.h"

using json = nlohmann::json;

nlohmann::json UnitSaveData::data;

bool UnitSaveData::SaveFileExists() {
    return std::filesystem::exists(SAVE_FOLDER_PATH + "unit_save.json");
}
void UnitSaveData::CreateSaveData() {
    data["units"] = json::array();

    json unitData;

    unitData["level"] = 1;
    unitData["unlocked_gear"] = 0; //  0 = NOT unlocked, 1+ = unlocked and gear #
    unitData["cores"] = json::array();

    for (int i = 0; i < UnitConfig::MAX_CORES; i++)
        unitData["cores"].push_back(false);

    // Push the default data for each player unit
    for (int id = 0; id < UnitConfig::TOTAL_PLAYER_UNITS; id++) {
        unitData["unlocked_gear"] = UnitConfig::getMaxGear(id, true); // auto unlock
        data["units"].push_back(unitData);
    }

    // set the first Unit as unlocked
   // data["units"][0]["unlocked_gear"] = 1;

    std::ofstream file(SAVE_FOLDER_PATH + "unit_save.json");
    file << data.dump(2);  // The '2' means indent with 2 spaces
    file.close();
}
void UnitSaveData::Load() {
    std::ifstream file(SAVE_FOLDER_PATH + "unit_save.json");
    data = json::parse(file);
}
void UnitSaveData::Save() {
    std::ofstream file(SAVE_FOLDER_PATH + "unit_save.json");
    file << data.dump(2);
}

int UnitSaveData::GetLevel(int unitID) { return data["units"][unitID]["level"]; }
int UnitSaveData::IncreaseLevel(int unitID) {
    int newLevel = data["units"][unitID]["level"] + 1;
    data["units"][unitID]["level"] = newLevel;

    Save(); 

    return newLevel;
}
int UnitSaveData::GetMaxGear(int unitID) { return data["units"][unitID]["unlocked_gear"].get<int>();}
void UnitSaveData::SetNewGear(int unitID, int newGear) {
    data["units"][unitID]["unlocked_gear"] = newGear;
    Save();
}
bool UnitSaveData::HasCore(int unitID, int coreID) { return data["units"][unitID]["cores"][coreID]; }
void UnitSaveData::UnlockCore(int unitID, int coreID) {
    data["units"][unitID]["cores"][coreID] = true;
    Save();
}
