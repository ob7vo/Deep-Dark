#pragma once
#include "UnitEnums.h"
#include "json.hpp"
#include <fstream>

namespace ProjData {
	std::string get_proj_path(int id);
	nlohmann::json get_proj_json(int id);
};

struct ProjectileStats {
	int team = 0;
	int hits = 0;
	float maxLifespan = 999.f;

	int dmg = 0;
	float height = 0;
	float width = 0;

	Augment aug = {};

	ProjectileStats() = default;
	void setup(const nlohmann::json& projJson);
};