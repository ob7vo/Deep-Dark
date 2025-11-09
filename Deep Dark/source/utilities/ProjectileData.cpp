#include "Projectile.h"
#include <algorithm>

namespace ProjData {
    std::string get_proj_path(int id) {
        return std::format("configs/proj_data/{}/", id);
    }
    nlohmann::json get_proj_json(int id) {
        const std::string path = get_proj_path(id) + "proj_data.json";
        //std::cout << path << std::endl;
        std::ifstream file(path);

        try {
            return nlohmann::json::parse(file);
        }
        catch (const nlohmann::json::parse_error& e) {
            std::cerr << "JSON parse error in " << path << ": " << e.what() << std::endl;
            return nlohmann::json();
        }
    }
};

void ProjectileStats::setup(const nlohmann::json& projJson) {
    team = projJson["stats"]["team"];
    hits = projJson["stats"].value("hits", 10);

    dmg = projJson["stats"]["dmg"];
    height = projJson["stats"]["height"];
    width = projJson["stats"]["width"];

    if (projJson.contains("augment")) {
        AugmentType augType = Augment::string_to_augment_type(projJson["augment"]["type"]);
        if (augType == NONE) return;
        aug = Augment::from_json(augType, projJson);
    }
}