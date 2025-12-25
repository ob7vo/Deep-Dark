#include "pch.h"
#include "Base.h"
#include "Stage.h"
#include "BaseTextures.h"

using json = nlohmann::json;

static AnimationClip waveCannonAni;

const float MAX_FIRE_WALL_DIST = 10000.f;
const float FIRE_WALL_SPAWN_PADDING = 7.5f;

const float ORBITAL_CANNON_SPACING = 45.f;
const int ORBITAL_STRIKES = 6;
const int BOTTOM_LANE = 0;

const float SHORT_CIRCUIT_CANNON_RANGE = 300.f;
const int zero = 0;

#pragma region Constructos
BaseCannon::BaseCannon(const json& baseJson, float magnification) :
	cannonStats(UnitStats::create_cannon(baseJson, magnification)), team(baseJson["team"]) {}

WaveCannon::WaveCannon(const json& baseJson, float magnification) :BaseCannon(baseJson, magnification), 
shockWave(Augment::cannon(AugmentType::SHOCK_WAVE, baseJson["surge_level"])) {}

FireWallCannon::FireWallCannon(const json& baseJson, float magnification) :BaseCannon(baseJson, magnification), 
fireWall(Augment::cannon(AugmentType::FIRE_WALL, baseJson["surge_level"])) {}

OrbitalCannon::OrbitalCannon(const json& baseJson, float magnification) :BaseCannon(baseJson, magnification),
orbitalStrike(Augment::cannon(AugmentType::ORBITAL_STRIKE,1)) {}

AreaCannon::AreaCannon(const json& baseJson, float magnification) : BaseCannon(baseJson, magnification),
areaRange(baseJson["area_range"]) {}
#pragma endregion

#pragma region Cannon Fire
void WaveCannon::fire(Stage& stage) {
	stage.create_surge(this, shockWave);
}

void FireWallCannon::fire(Stage& stage) {
	for (int i = 0; stage.laneCount; i++) {
		auto& playerUnits = stage.lanes[i].getOpponentUnits(-1);

		for (auto it = playerUnits.begin(); it != playerUnits.end();) {
			float distance = std::abs(pos.x - it->get_pos().x);
			
			if (distance < MAX_FIRE_WALL_DIST) {
				stage.create_surge(this, fireWall, i, distance + FIRE_WALL_SPAWN_PADDING);
				break;
			}
		}
	}
}

void OrbitalCannon::fire(Stage& stage) {
	for (int i = 1; i <= ORBITAL_STRIKES; i++) {
		float dist = ORBITAL_CANNON_SPACING * (float)i;

		stage.create_surge(this, orbitalStrike, BOTTOM_LANE, dist);
	}
}

void AreaCannon::fire(Stage& stage) {
	for (int i = 0; i < stage.laneCount; i++) {
		std::vector<Unit>& enemyUnits = stage.lanes[i].getOpponentUnits(cannonStats.team);

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

static AnimationClip base_animation() {
	sf::Vector2i cellSize = { 96,64 };
	sf::Vector2f origin = { 48,64 };

	return AnimationClip(&Textures::Base::t_base2, 21, .15f, cellSize, origin, {{15, AnimationEvent::ATTACK}}, false);
}
void WaveCannon::create_animation() {animClip = base_animation(); }
void FireWallCannon::create_animation() { animClip = base_animation(); }
void OrbitalCannon::create_animation() { animClip = base_animation(); }
void AreaCannon::create_animation() { animClip = base_animation(); }

