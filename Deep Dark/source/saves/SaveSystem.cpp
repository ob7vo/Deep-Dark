#include "pch.h"
#include "SaveSystem.h"
#include "PlayerSaveData.h"
#include "StageSaveData.h"
#include "UnitSaveData.h"
#include "UnitConfig.h"
#include "StageConfig.h"

using json = nlohmann::json;

void SaveSystem::Initialize(bool overwritePlayerSave, bool overwriteStageSave, bool overwriteUnitSave) {
    if (!PlayerSaveData::SaveFileExists() || overwritePlayerSave)
        PlayerSaveData::CreateSaveData();
    else PlayerSaveData::Load();

    if (!StageSaveData::SaveFileExists() || overwriteStageSave)
        StageSaveData::CreateSaveData();
    else StageSaveData::Load();

    if (!UnitSaveData::SaveFileExists() || overwriteUnitSave) 
        UnitSaveData::CreateSaveData();
    else UnitSaveData::Load();
}


void SaveSystem::LoadAll() {
    PlayerSaveData::Load();
    StageSaveData::Load();
    UnitSaveData::Load();
}
void SaveSystem::SaveAll() {
    PlayerSaveData::Save();
    StageSaveData::Save();
    UnitSaveData::Save();
}
