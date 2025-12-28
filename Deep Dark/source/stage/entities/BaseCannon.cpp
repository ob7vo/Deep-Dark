#include "pch.h"
#include "Base.h"
#include "Stage.h"
#include "BaseTextures.h"
#include "EntityConfigs.h"

using json = nlohmann::json;

static AnimationClip waveCannonAni;

const int ZERO = 0;

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
		// Attack players only
		for (const auto& index : stage.lanes[i].getOpponentUnits(-1)) {
			float distance = std::abs(pos.x - stage.getUnit(index).get_pos().x);
			
			if (distance < EntityConfigs::Bases::MAX_FIRE_WALL_DIST) {
				stage.create_surge(this, fireWall, i, distance + EntityConfigs::Bases::FIRE_WALL_SPAWN_PADDING);
				break;
			}
		}
	}
}

void OrbitalCannon::fire(Stage& stage) {
	for (int i = 1; i <= EntityConfigs::Bases::ORBITAL_STRIKES; i++) {
		float dist = EntityConfigs::Bases::ORBITAL_CANNON_SPACING * (float)i;

		stage.create_surge(this, orbitalStrike, EntityConfigs::Bases::BOTTOM_LANE, dist);
	}
}

void AreaCannon::fire(Stage& stage) {
	for (const auto& lane : stage.lanes) {
		const std::vector<size_t>& enemyUnitIndexes = lane.getOpponentUnits(cannonStats.team);

		for (const auto& index : enemyUnitIndexes) {
			auto& enemyUnit = stage.getUnit(index);

			if (is_valid_target(enemyUnit))
				enemyUnit.status.apply_effects(enemyUnit, cannonStats.augments, 0);
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

