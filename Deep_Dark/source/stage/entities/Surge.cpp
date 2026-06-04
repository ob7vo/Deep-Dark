#include "pch.h"
#include "Surge.h"
#include "Tween.h"
#include "Stage.h"
#include "Unit.h"
#include "EntityTextures.h"
#include "EntityConfigs.h"

std::array<AnimationClip, 2> ShockWave::shockWaveAni;
std::array<AnimationClip, 3> FireWall::fireWallAni;
AnimationClip OrbitalStrike::orbitalStrikeAni;

using namespace EntityConfigs::Surges;

#pragma region Constructors
Surge::Surge(const UnitStats* stats, int curLane, sf::Vector2f pos) :
	StageEntity(pos, curLane), stats(stats), animationState(SurgeAnimationStates::ACTIVE)
{
	hitUnits.reserve(12);
	sprite.setPosition(pos);
	hitbox.setPosition(pos);
	hitbox.setFillColor(sf::Color(240, 48, 38, 100));
}

ShockWave::ShockWave(const UnitStats* stats, int curLane, int level, sf::Vector2f pos, Stage& stage) :
	Surge(stats, curLane, pos), id(stage.nextUnitID++) 
{
	halfWidth = SHOCK_WAVE_WIDTH * 0.5f;
	hitbox.setSize({ SHOCK_WAVE_WIDTH, 25.f });
	hitbox.setOrigin({ halfWidth, 25.f });

	// creating the Tween
	sf::Vector2f endPos = pos;
	endPos.x += (SHOCK_WAVE_Lv1_DISTANCE + (SHOCK_WAVE_DISTANCE_PER_LEVEL * (float)level - 1)) * (float)stats->team;
	float timeLeft = SHOCK_WAVE_Lv1_TIMER + (SHOCK_WAVE_TIME_PER_LEVEL * (float)level - 1);

	tween.start(pos, endPos, timeLeft);

	animationState = SurgeAnimationStates::ACTIVE;
	animPlayer.start(&shockWaveAni[0], sprite);
}
FireWall::FireWall(const UnitStats* stats, int curLane, int level, sf::Vector2f pos) :
	Surge(stats, curLane, pos), level(level)
{
	permanentHitUnits.reserve(15);

	halfWidth = FIREWALL_WIDTH * 0.5f;
	hitbox.setSize({ FIREWALL_WIDTH, 30.f });
	hitbox.setOrigin({ halfWidth, 30.f });

	timeLeft = FIREWALL_TIMER_PER_LEVEL;

	animationState = SurgeAnimationStates::START_UP;
	animPlayer.start(&fireWallAni[0], sprite);
}
OrbitalStrike::OrbitalStrike(const UnitStats* stats, int curLane, sf::Vector2f pos) :
	Surge(stats, curLane, pos)
{
	halfWidth = ORBITAL_STRIKE_WIDTH * 0.5f;
	hitbox.setSize({ ORBITAL_STRIKE_WIDTH, 150.f });
	hitbox.setOrigin({ ORBITAL_STRIKE_WIDTH, 75.f });

	animationState = SurgeAnimationStates::ACTIVE;
	animPlayer.start(&orbitalStrikeAni, sprite);
}
#pragma endregion

// Animation functions
void ShockWave::start_animation(SurgeAnimationStates newState) {
	animationState = newState;
	animPlayer.start(&shockWaveAni[static_cast<int>(animationState) - 1], sprite);
};
void FireWall::start_animation(SurgeAnimationStates newState) {
	animationState = newState;
	animPlayer.start(&fireWallAni[static_cast<int>(newState)], sprite);
};
AnimationEvent ShockWave::update_animation(Stage& stage, float deltaTime) {
	auto events = animPlayer.update(deltaTime, sprite);
	sprite.setPosition(pos);
	hitbox.setPosition(pos);

	return events;
}
AnimationEvent Surge::update_animation(Stage& stage, float deltaTime) {
	return animPlayer.update(deltaTime, sprite);
}

//Check
bool Surge::valid_target(const Unit& unit) const {
	// Short handing conditions forif an enemy unit can be attacked
	return !already_hit_unit(unit.spawnID) && !unit.anim.invincible()
		&& in_range(unit.movement.pos.x);
}

#pragma region Combat
bool Surge::try_terminate_unit(const Unit& enemyUnit, int dmg) const{
	if (!stats->has_augment(AugmentType::TERMINATE)) return false;

	// Kill unit if under the health threshold
	float threshold = stats->get_augment(AugmentType::TERMINATE)->data.onHPThreshold.hpPercentage;
	float curHpPercent = (float)(enemyUnit.status.hp - dmg) / (float)enemyUnit.stats->maxHp;

	return curHpPercent <= threshold;
}
void Surge::on_kill(Unit& unit) const {
	if (unit.stats->try_proc_augment(AugmentType::PLUNDER))
		unit.status.statusFlags |= AugmentType::PLUNDER;
	if (stats->has_augment(AugmentType::CODE_BREAKER)) 
		unit.status.statusFlags |= AugmentType::CODE_BREAKER;

	unit.causeOfDeath |= createdByCannon ? DeathCause::CANNON : DeathCause::SURGE;
};
void Surge::attack_units(Stage& stage) {
	// lane is froms tage (passed in)
	const auto& enemyUnitIndexes = stage.lanes[laneIdx].getOpponentUnits(stats->team);
	std::vector<size_t> enemiesToAttack;

	// Add in units to attack AFTER this loop to make Surge Blocking
	// more consistent and reliable
	for (const auto& index : enemyUnitIndexes)
	{
		const auto& enemyUnit = stage.getUnit(index); // don't have stage

		if (!valid_target(enemyUnit)) continue;

		if (enemyUnit.immune(surgeType)) {
			// if the surge is blocked, remove all
			if (blocked_by_unit(enemyUnit)) {
				enemiesToAttack.clear();
				readyForRemoval = true;
				return;
			}
			else continue;
		}
		else enemiesToAttack.push_back(index);
	}

	// Attack the valid enemies
	for (const auto& index : enemiesToAttack) {
		auto& enemyUnit = stage.getUnit(index);

		hitUnits.push_back(enemyUnit.spawnID);
		if (enemyUnit.status.take_damage(*this))
			on_kill(enemyUnit);
	}
}
int Surge::calculate_damage_taken_and_apply_augments(Unit& unit) const {
	float dmg = (float)get_dmg();

	dmg *= unit.status.get_corrosion_multiplier();
	dmg *= unit.status.get_reinforcement_multiplier();

	// If the unit targets the surge's trait, run its defense-augments
	if (targeted_by_unit(unit.stats->targetTypes))
		for (const Augment& augment : unit.stats->augments) {
			if (has(augment.augType & AugmentType::RESIST))
				dmg *= augment.data.damage.dmgMultiplier;
		}


	if (unit.is_targeted(stats->targetTypes)) {
		unit.status.apply_on_hit_status_effects(stats->augments, hitIndex);
		dmg *= unit.status.calculate_damage_boost(stats->augments);

		// These effects are based around the Unit's current HP, 
		// so they are run after all calculations

		if (stats->try_proc_augment(AugmentType::VOID, hitIndex))
			dmg += (float)unit.stats->maxHp * stats->get_augment(AugmentType::VOID)->data.general.magnitude;
		if (try_terminate_unit(unit, dmg))
			dmg += (float)unit.stats->maxHp;
	}

	return (int)dmg;
}
#pragma endregion

// TICK FUNCTIONS //////////////////////////////////////////////////
#pragma region Tick Functions
void ShockWave::tick(Stage& stage, float deltaTime) {
	auto events = update_animation(stage, deltaTime);
	attack_units(stage);

	if (animationState == SurgeAnimationStates::ACTIVE) {
		if (tweening())
			start_animation(SurgeAnimationStates::ENDING);
		else {
			tween.update(pos, deltaTime);
			if (!tweening()) cancel_tween();
		}
	}
	else if (animationState == SurgeAnimationStates::ENDING)
		readyForRemoval = any(events & AnimationEvent::FINAL_FRAME);

}
void FireWall::tick(Stage& stage, float deltaTime) {
	auto events = update_animation(stage, deltaTime);

	switch (animationState) {
	case SurgeAnimationStates::START_UP:
		if (any(events & AnimationEvent::FINAL_FRAME))
			start_animation(SurgeAnimationStates::ACTIVE);
		break;
	case SurgeAnimationStates::ACTIVE:
		timeLeft -= deltaTime;
		attack_units(stage);

		if (timeLeft < 0) {
			timeLeft = FIREWALL_TIMER_PER_LEVEL;
			if (--level > 0) {
				permanentHitUnits.insert(permanentHitUnits.end(), hitUnits.begin(), hitUnits.end());
				hitUnits.clear();
			}
			else start_animation(SurgeAnimationStates::ENDING);
		}
		break;
	case SurgeAnimationStates::ENDING:
		if (any(events & AnimationEvent::FINAL_FRAME))
			readyForRemoval = true;
	}
}
void OrbitalStrike::tick(Stage& stage, float deltaTime){
	auto events = update_animation(stage, deltaTime);

	if (any(events & AnimationEvent::ATTACK))
		for (int i = 0; i < stage.laneCount; i++)
			attack_units(stage);
	else if (any(events & AnimationEvent::FINAL_FRAME))
		readyForRemoval = true;
}
#pragma endregion
// END OF TICK FUNCTIONS ///////////////////////////////////////////

bool FireWall::never_hit_unit(int id) {
	return !already_hit_unit(id) && 
		std::find(permanentHitUnits.begin(), permanentHitUnits.end(), id) == permanentHitUnits.end();
}
void Surge::init_animations() {
	// SHOCK WAVE
	sf::Vector2i cellSize = { 32,32 };
	sf::Vector2f origin = { 16, 32 };
	ShockWave::shockWaveAni[0] = AnimationClip(&Textures::Entity::t_shockWaveActive, 
		10, .1f, cellSize, origin, {}, true);
	ShockWave::shockWaveAni[1] = AnimationClip(&Textures::Entity::t_shockWaveEnding,
		8, .1f, cellSize, origin);

	// FIRE WALL
	FireWall::fireWallAni[0] = AnimationClip(&Textures::Entity::t_fireWallStartUp,
		12, .05f, cellSize, origin);
	FireWall::fireWallAni[1] = AnimationClip(&Textures::Entity::t_fireWallActive,
		8, .1f, cellSize, origin, {}, true);
	FireWall::fireWallAni[2] = AnimationClip(&Textures::Entity::t_fireWallEnding, 
		9, .05f, cellSize, origin);

	// ORBITAL STRIKE
	sf::Vector2i os_cellSize = { 96,96 };
	origin = { 48,48 };
	OrbitalStrike::orbitalStrikeAni = AnimationClip(&Textures::Entity::t_orbitalStrikeActive
		, 12, .2f, os_cellSize, origin, {{4, AnimationEvent::ATTACK}});
}

#pragma region Common Utility Functions
void Surge::draw(sf::RenderWindow& window) const {
	window.draw(sprite);
	window.draw(hitbox);
}

bool Surge::blocked_by_unit(const Unit& enemyUnit) const {
	return enemyUnit.enemy_in_range(pos.x, 0, enemyUnit.stats->sightRange) &&
		has(enemyUnit.stats->augmentsMask, AugmentType::SURGE_BLOCKER);
}
bool Surge::already_hit_unit(int id) const {
	return std::find(hitUnits.begin(), hitUnits.end(), id) != hitUnits.end();
}
bool Surge::targeted_by_unit(UnitType enemyTargetTypes) const { return stats->is_targeted(enemyTargetTypes); }
bool Surge::in_range(float x) const {
	return x >= pos.x - halfWidth && x <= pos.x + halfWidth;
}
int Surge::get_dmg() const { return stats->get_hit_stats(hitIndex).dmg; }
#pragma endregion