#include "pch.h"
#include "PlayerSaveData.h"
#include "SaveSystem.h"

using json = nlohmann::json;

nlohmann::json PlayerSaveData::data;

bool PlayerSaveData::SaveFileExists() {
    return std::filesystem::exists(SAVE_FOLDER_PATH + "player_save.json");
}
void PlayerSaveData::CreateSaveData() {
    data["scrap_parts"] = 0; // Currency for Levels. From clearing stages
    data["modules"] = 0; // Currency for True Forms and Cores. From Challenges

    data["playtime"] = 0.f;
    data["stages_cleared"] = 0;
    data["challenges_cleared"] = 0;

    std::ofstream file(SAVE_FOLDER_PATH + "player_save.json");
    file << data.dump(2);  // The '2' means indent with 2 spaces
    file.close();
}

void PlayerSaveData::Load() {
    std::ifstream file(SAVE_FOLDER_PATH + "player_save.json");
    data = json::parse(file);
    file.close();
}
void PlayerSaveData::Save() {
    std::ofstream file(SAVE_FOLDER_PATH + "player_save.json");
    file << data.dump(2);
    file.close();
}

int PlayerSaveData::GetScrapParts() {
    return data["scrap_parts"].get<int>();
}
std::optional<int> PlayerSaveData::SpendScrapParts(int cost) {
    int curScrapParts = data["scrap_parts"].get<int>();

    if (cost >= curScrapParts) {
        data["scrap_parts"] = std::max(curScrapParts - cost, 0);
        Save();

        return data["scrap_parts"].get<int>();;
    }
    else return std::nullopt;
}
int PlayerSaveData::GainScrapParts(int partsToGain) {
    int scrapParts = data["scrap_parts"].get<int>();
    
    scrapParts = std::min(scrapParts + partsToGain, MAX_SCRAP_PARTS);
    data["scrap_parts"] = scrapParts;
    Save();

    return scrapParts;
}
int PlayerSaveData::GetModules() {
    return data["modules"].get<int>();
}
std::optional<int> PlayerSaveData::SpendModules(int cost) {
    int curModules = data["modules"].get<int>();

    if (cost >= curModules) {
        data["modules"] = std::max(curModules - cost, 0);
        Save();

        return data["modules"].get<int>();;
    }
    else return std::nullopt;
}
int PlayerSaveData::GainModules(int modsToGain) {
    int modules = data["modules"].get<int>();

    // I just use max scrap for bounding since it doesnt matter.
    modules = std::min(modules + modsToGain, MAX_SCRAP_PARTS);
    data["modules"] = modules;
    Save();

    return modules;
}
void PlayerSaveData::AddStageClear() {
    data["stages_cleared"] = data["stages_cleared"].get<int>() + 1;
    Save();
}
void PlayerSaveData::AddChallengeClears(int newClears) {
    data["challenges_cleared"] = data["challenges_cleared"].get<int>() + newClears;
    Save();
}