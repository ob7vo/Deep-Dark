#include "SaveSystem.h"
#include "UnitData.h"

using json = nlohmann::json;

SaveSystem SaveSystem::s_instance;

void SaveSystem::initialize() {
	if (!saveFileExists("stage_save.json"))
		create_stage_save();
	if (!saveFileExists("unit_save.json"))
		create_unit_save();
}
void SaveSystem::create_unit_save() {
    json data;
    data["modules_spent"] = 0;

    // each array will have a size of const UNITS

    // array of ints, each index has a number from -1 - 2
    // -1 = not unlocked, 0 = only base form, 1 = evolved form, 2 = has true form
    data["unlocked_units"] = json::array();
    data["unlocked_units"].emplace_back(0);
    data["unit_cores"] = json::array();
    for (int i = 1; i < UnitData::TOTAL_UNITS; i++) {
        data["unlocked_units"].push_back(-1);
        data["unit_cores"].push_back({ false, false, false });
    }

    // array of boolean arrays. Each bool array will have max 3 bools
    // Every Unit and ID has cores, but only in their FINAL form. 
    // so gears are irrelevant here

    std::ofstream file(SAVE_FOLDER_PATH + "unit_save.json");
    file << data.dump(2);  // The '2' means indent with 2 spaces
    file.close();
}
void SaveSystem::create_stage_save() {

}
void SaveSystem::load_save(const std::string file) {

}
