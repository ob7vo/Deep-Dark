#include "pch.h"
#include "Surge.h"
#include "Tween.h"
#include "Stage.h"
#include "Unit.h"
#include "UnitData.h"

std::array<Animation, 2> ShockWave::shockWaveAni;
std::array<Animation, 3> FireWall::fireWallAni;
Animation OrbitalStrike::orbitalStrikeAni;

#pragma region Constructors
Surge::Surge(const UnitStats* stats, int curLane, sf::Vector2f pos) :
	stats(stats), currentLane(curLane), pos(pos), animationState(SurgeAnimationStates::ACTIVE)
{
	hitUnits.reserve(12);
	sprite.setPosition(pos);
	hitbox.setPosition(pos);
	hitbox.setFillColor(sf::Color(240, 48, 38, 100));
}

ShockWave::ShockWave(const UnitStats* stats, int curLane, int level, sf::Vector2f pos, Stage& stage) :
	Surge(stats, curLane, pos), id(stage.nextUnitID++) 
{
	halfWidth = SW_WIDTH * 0.5f;
	hitbox.setSize({ SW_WIDTH, 25.f });
	hitbox.setOrigin({ halfWidth, 25.f });

	// creating the Tween
	sf::Vector2f endPos = pos;
	endPos.x += (SW_BASE_DISTANCE + (SW_DISTANCE_PER_LEVEL * (float)level - 1)) * (float)stats->team;
	float timeLeft = SW_TWEEN_TIMER + (SW_TWEEN_TIME_PER_LEVEL * (float)level - 1);

	tween.start(pos, endPos, timeLeft);

	animationState = SurgeAnimationStates::ACTIVE;
	shockWaveAni[0].reset(aniTime, currentFrame, sprite);
}
FireWall::FireWall(const UnitStats* stats, int curLane, int level, sf::Vector2f pos) :
	Surge(stats, curLane, pos), level(level)
{
	permanentHitUnits.reserve(15);
	halfWidth = FW_WIDTH * 0.5f;
	hitbox.setSize({ FW_WIDTH, 30.f });
	hitbox.setOrigin({ halfWidth, 30.f });
	timeLeft = FW_TIMER;

	animationState = SurgeAnimationStates::START_UP;
	fireWallAni[0].reset(aniTime, currentFrame, sprite);
}
OrbitalStrike::OrbitalStrike(const UnitStats* stats, int curLane, sf::Vector2f pos) :
	Surge(stats, curLane, pos)
{
	halfWidth = OS_WIDTH * 0.5f;
	hitbox.setSize({ OS_WIDTH, 150.f });
	hitbox.setOrigin({ halfWidth, 75.f });

	animationState = SurgeAnimationStates::ACTIVE;
	orbitalStrikeAni.reset(aniTime, currentFrame, sprite);
}
#pragma endregion

// Deconstructors
Surge::~Surge() = default;
ShockWave::~ShockWave() = default;
FireWall::~FireWall() = default;
OrbitalStrike::~OrbitalStrike() = default;

// Animation functions
void ShockWave::start_animation(SurgeAnimationStates newState) {
	animationState = newState;
	shockWaveAni[static_cast<int>(newState) - 1].reset(aniTime, currentFrame, sprite);
};
void FireWall::start_animation(SurgeAnimationStates newState) {
	animationState = newState;
	fireWallAni[static_cast<int>(newState)].reset(aniTime, currentFrame, sprite);
};
int ShockWave::update_animation( float deltaTime) {
	auto events = shockWaveAni[static_cast<int>(animationState) - 1].update(aniTime, currentFrame, deltaTime, sprite);
	sprite.setPosition(pos);
	hitbox.setPosition(pos);

	return events;
}
int FireWall::update_animation(float deltaTime) {
	return fireWallAni[static_cast<int>(animationState)].update(aniTime, currentFrame, deltaTime, sprite);
}
int OrbitalStrike::update_animation(float deltaTime) {
	return orbitalStrikeAni.update(aniTime, currentFrame, deltaTime, sprite);
}

//Check
bool Surge::valid_target(const Unit& unit) const {
	return !already_hit_unit(unit.id) && !unit.anim.invincible()
		&& in_range(unit.get_pos().x);
}

#pragma region Combat
bool Surge::try_terminate_unit(const Unit& enemyUnit, int dmg) const{
	if (!stats->has_augment(TERMINATE)) return false;
	float threshold = stats->get_augment(TERMINATE).value;
	float curHpPercent = (float)(enemyUnit.status.hp - dmg) / (float)enemyUnit.stats->maxHp;

	return curHpPercent <= threshold;
}
void Surge::on_kill(Unit& unit) const {
	if (unit.stats->try_proc_augment(PLUNDER)) unit.status.statusFlags |= PLUNDER;
	if (stats->has_augment(CODE_BREAKER)) unit.status.statusFlags |= CODE_BREAKER;

	unit.causeOfDeath = createdByCannon ? DeathCause::CANNON : DeathCause::SURGE;
};
void Surge::attack_units(Lane& lanes) {
	std::vector<Unit>& enemyUnits = lanes.getOpponentUnits(stats->team);

	for (auto it = enemyUnits.begin(); it != enemyUnits.end(); ++it) {
		if (!valid_target(*it)) continue;

		if (immune_to_surge_type(it->stats->immunities)) {
			if (it->stats->surge_blocker()) {
				Tween::cancel(&pos);
				readyForRemoval = true;
				return;
			}
			else continue;
		}

		hitUnits.push_back(it->id);
		if (it->status.take_damage(*it, *this)) on_kill(*it);
	}
}
int Surge::calculate_damage_and_effects(Unit& unit) const {
	int dmg = get_dmg();
	dmg = unit.status.corroded() ? dmg * 2 : dmg;

	// If the surge targets the unit's trait, run its damage-augments
	if (unit.targeted_by_unit(stats->targetTypes))
		dmg = unit.status.apply_effects(unit, stats->augments, hitIndex, dmg);

	// If the unit targets the surge's trait, run its defense-augments
	if (targeted_by_unit(unit.stats->targetTypes))
		dmg = (int)((float)dmg * unit.status.calculate_damage_reduction(unit.stats->augments));

	// if the unit has a sheild and it did not break, then return.
	if (unit.status.has_shield_up() && !unit.status.damage_shield(dmg, stats)) return 0;

	// These effects are based around the Unit's current HP, 
	// so they are run after all calculations
	if (unit.targeted_by_unit(stats->targetTypes)) {
		if (stats->try_proc_augment(VOID, hitIndex))
			dmg += (int)((float)unit.stats->maxHp * stats->get_augment(VOID).value);
		if (try_terminate_unit(unit, dmg))
			dmg += unit.stats->maxHp;
	}

	return dmg;
}
#pragma endregion

// TICK FUNCTIONS //////////////////////////////////////////////////
#pragma region Tick Functions
void ShockWave::tick(float deltaTime, Stage& stage) {
	auto events = update_animation(deltaTime);
	attack_units(stage.lanes[currentLane]);

	if (animationState == SurgeAnimationStates::ACTIVE) {
		if (tweening())
			start_animation(SurgeAnimationStates::ENDING);
		else {
			tween.update(pos, deltaTime);
			if (!tweening()) cancel_tween();
		}
	}
	else if (animationState == SurgeAnimationStates::ENDING)
		readyForRemoval = events & FINAL_FRAME;

}
void FireWall::tick(float deltaTime, Stage& stage) {
	auto events = update_animation(deltaTime);

	switch (animationState) {
	case SurgeAnimationStates::START_UP:
		if (Animation::check_for_event(AnimationEvent::FINAL_FRAME, events))
			start_animation(SurgeAnimationStates::ACTIVE);
		break;
	case SurgeAnimationStates::ACTIVE:
		timeLeft -= deltaTime;
		attack_units(stage.lanes[currentLane]);

		if (timeLeft < 0) {
			timeLeft = FW_TIMER;
			if (--level > 0) {
				permanentHitUnits.insert(permanentHitUnits.end(), hitUnits.begin(), hitUnits.end());
				hitUnits.clear();
			}
			else start_animation(SurgeAnimationStates::ENDING);
		}
		break;
	case SurgeAnimationStates::ENDING:
		if (Animation::check_for_event(AnimationEvent::FINAL_FRAME, events)) 
			readyForRemoval = true;
	}
}
void OrbitalStrike::tick(float deltaTime, Stage& stage)  {
	auto events = update_animation(deltaTime);

	if (Animation::check_for_event(AnimationEvent::ATTACK, events))
		for (int i = 0; i < stage.laneCount; i++)
			attack_units(stage.lanes[i]);
	else if (Animation::check_for_event(AnimationEvent::FINAL_FRAME, events))
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
	std::string spritePath = "sprites/surges/wave_active.png";
	sf::Vector2i cellSize = { 32,32 };
	sf::Vector2f origin = { 16, 32 };
	ShockWave::shockWaveAni[0] = Animation(spritePath, 10, .1f, cellSize, origin, {}, true);
	spritePath = "sprites/surges/wave_ending.png";
	ShockWave::shockWaveAni[1] = Animation(spritePath, 8, .1f, cellSize, origin);

	// FIRE WALL
	spritePath = "sprites/surges/surge_start_up.png";
	FireWall::fireWallAni[0] = Animation(spritePath, 12, .05f, cellSize, origin);
	spritePath = "sprites/surges/surge_active.png";
	FireWall::fireWallAni[1] = Animation(spritePath, 8, .1f, cellSize, origin, {}, true);
	spritePath = "sprites/surges/surge_ending.png";
	FireWall::fireWallAni[2] = Animation(spritePath, 9, .05f, cellSize, origin);

	// ORBITAL STRIKE
	spritePath = "sprites/surges/orbital_strike_active.png";
	sf::Vector2i os_cellSize = { 96,96 };
	origin = { 48,48 };
	OrbitalStrike::orbitalStrikeAni = Animation(spritePath, 12, .2f, os_cellSize, origin, {{4, ATTACK}});
}

#pragma region Common Utility Functions
void Surge::draw(sf::RenderWindow& window) const {
	window.draw(sprite);
	window.draw(hitbox);
}

bool Surge::already_hit_unit(int id) const {
	return std::find(hitUnits.begin(), hitUnits.end(), id) != hitUnits.end();
}
bool Surge::immune_to_surge_type(size_t unitImmunities) const { return surgeType & unitImmunities; }
bool Surge::targeted_by_unit(int enemyTargetTypes) const { return stats->targeted_by_unit(enemyTargetTypes); }
bool Surge::in_range(float x) const {
	return x >= pos.x - halfWidth && x <= pos.x + halfWidth;
}
int Surge::get_dmg() const { return stats->get_hit_stats(hitIndex).dmg; }
#pragma endregion