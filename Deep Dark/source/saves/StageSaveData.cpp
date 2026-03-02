#include "pch.h"
#include "StageSaveData.h"
#include "SaveSystem.h"
#include "PlayerSaveData.h"
#include "StageConfig.h"

using json = nlohmann::json;

nlohmann::json StageSaveData::data;

bool StageSaveData::SaveFileExists() {
    return std::filesystem::exists(SAVE_FOLDER_PATH + "stage_save.json");
}
void StageSaveData::CreateSaveData() {
    // Created the array of stages
    data["stages"] = json::array();

    // Start creating the data that each stage will need saved
    json stageData;

    stageData["is_unlocked"] = false;
    stageData["clears"] = 0;
    stageData["clear_time"] = 0.f;

    stageData["challenge_status"] = json::array();
    for (int i = 0; i < StageConfig::CHALLENGES_PER_STAGE; i++)
        stageData["challenge_status"].push_back(false);

    // Push the default data for each stage
    for (int i = 0; i < StageConfig::TOTAL_STAGES; i++)
        data["stages"].push_back(stageData);

    data["stages"][0]["is_unlocked"] = true;

    std::ofstream file(SAVE_FOLDER_PATH + "stage_save.json");
    file << data.dump(2);  // The '2' means indent with 2 spaces
    file.close();
}
void StageSaveData::Load() {
    std::ifstream file(SAVE_FOLDER_PATH + "stage_save.json");
    data = json::parse(file);
    file.close();
}
void StageSaveData::Save() {
    std::ofstream file(SAVE_FOLDER_PATH + "stage_save.json");
    file << data.dump(2);
    file.close();
}

int StageSaveData::GetStageClears(int stageID) { return data["stages"][stageID]["clears"];}
int StageSaveData::AddStageClear(int stageID) {
    int stageClears = data["stages"][stageID]["clears"] + 1;

    if (stageClears == 1) PlayerSaveData::AddStageClear();
    data["stages"][stageID]["clears"] = stageClears;

    return stageClears;
}
float StageSaveData::GetClearTime(int stageID) { return data["stages"][stageID]["clear_time"]; }
float StageSaveData::SetClearTime(int stageID, float newTime) {
    float oldTime = data["stages"][stageID]["clear_time"];

    if (oldTime <= newTime) return oldTime;
    else {
        data["stages"][stageID]["clear_time"] = newTime;
        return newTime;
    }
}
std::bitset<StageConfig::CHALLENGES_PER_STAGE> StageSaveData::GetChallengeStatus(int stageID) {
    std::bitset<StageConfig::CHALLENGES_PER_STAGE> result;
    auto challenges = data["stages"][stageID]["challenge_status"];

    for (int i = 0; i < StageConfig::CHALLENGES_PER_STAGE; i++) 
        result[i] = challenges[i].get<bool>();

    return result;
}
void StageSaveData::ClearChallenge(int stageID, int chalIndex) {
    data["stages"][stageID]["challenge_status"][chalIndex] = true;
}
void StageSaveData::UnlockStages(int stageID) {
    nlohmann::json stageJson = StageConfig::getStageData(stageID);

    for (const auto& adjacentStageID : stageJson["adjacent_stages"].get<std::vector<int>>()) {
        data["stages"][adjacentStageID]["is_unlocked"] = true;
    }
}
bool StageSaveData::StageIsUnlocked(int stageID) {
    return data["stages"][stageID]["is_unlocked"].get<bool>();
}