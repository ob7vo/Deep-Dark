#include "pch.h"
#include "Base.h"
#include "Stage.h"
#include "Animation.h"

using json = nlohmann::json;

static Animation waveCannonAni;

const float MAX_FIRE_WALL_DIST = 10000.f;
const float FIRE_WALL_SPAWN_PADDING = 7.5f;

const float ORBITAL_CANNON_SPACING = 45.f;
const int ORBITAL_STRIKES = 6;

const float SHORT_CIRCUIT_CANNON_RANGE = 300.f;
const int zero = 0;

#pragma region Constructos
BaseCannon::BaseCannon(const json& baseJson, float magnification) :
	cannonStats(UnitStats::create_cannon(baseJson, magnification)), team(baseJson["team"]) {}

WaveCannon::WaveCannon(const json& baseJson, float magnification) :BaseCannon(baseJson, magnification), 
shockWave(Augment::cannon(SHOCK_WAVE, baseJson["surge_level"])) {}

FireWallCannon::FireWallCannon(const json& baseJson, float magnification) :BaseCannon(baseJson, magnification), 
fireWall(Augment::cannon(FIRE_WALL, baseJson["surge_level"])) {}

OrbitalCannon::OrbitalCannon(const json& baseJson, float magnification) :BaseCannon(baseJson, magnification),
orbitalStrike(Augment::cannon(ORBITAL_STRIKE,1)) {}

AreaCannon::AreaCannon(const json& baseJson, float magnification) : BaseCannon(baseJson, magnification),
areaRange(baseJson["area_range"]) {}
#pragma endregion

#pragma region Cannon Fire
void WaveCannon::fire(Stage& stage) {
	stage.create_surge(this, shockWave);
}

void FireWallCannon::fire(Stage& stage) {
	for (int i = 0; stage.laneCount; i++) {
		float minDist = MAX_FIRE_WALL_DIST + 1;
		auto& playerUnits = stage.get_lane_targets(i, -1);

		for (auto it = playerUnits.begin(); it != playerUnits.end();) {
			float distance = std::abs(pos.x - it->get_pos().x);
			minDist = std::min(minDist, distance);
		}
		
		if (minDist < MAX_FIRE_WALL_DIST)
			stage.create_surge(this, fireWall, i, minDist + FIRE_WALL_SPAWN_PADDING);
	}
}

void OrbitalCannon::fire(Stage& stage) {
	float dist = ORBITAL_CANNON_SPACING;
	int lane = 0;
	for (int i = 0; i < ORBITAL_STRIKES; i++) {
		stage.create_surge(this, orbitalStrike, lane, dist);
		dist += ORBITAL_CANNON_SPACING;
	}
}

void AreaCannon::fire(Stage& stage) {
	for (size_t i = 0; i < stage.laneCount; i++) {
		std::vector<Unit>& enemyUnits = stage.get_lane_targets((int)i, cannonStats.team);

		for (auto it = enemyUnits.begin(); it != enemyUnits.end();) {
			if (is_valid_target(*it))
				it->status.apply_effects(*it, cannonStats.augments, 0);
		}
	}
}
#pragma endregion

bool AreaCannon::is_valid_target(const Unit& target) const {
	return !target.anim.invincible() && within_range(target.get_pos().x);
}

static Animation base_animation() {
	std::string path = "sprites/bases/toxic_base.png";
	sf::Vector2i cellSize = { 96,64 };
	sf::Vector2f origin = { 48,64 };

	return Animation(path, 21, .15f, cellSize, origin, {{15, ATTACK}});
}
Animation WaveCannon::create_animation() { return base_animation(); }
Animation FireWallCannon::create_animation() { return base_animation(); }
Animation OrbitalCannon::create_animation() { return base_animation(); }
Animation AreaCannon::create_animation() { return base_animation(); }

