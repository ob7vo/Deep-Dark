#include "pch.h"
#include "Unit.h"
#include "Stage.h"
#include "Utils.h"
#include "StageRecord.h"

const float RUST_TYPE_LEDGE_RANGE = 10.f;
const float BASE_PHASING_TIMER = 1.f; // over 50 distance
const float PHASE_GAP_PADDING = 25; // Distance from gaps to assure Units dont immediately fall

Unit::Unit(Stage* curStage, sf::Vector2f startPos, int startingLane, const UnitStats* data,
	UnitAniMap* aniMap, int id) : 
	stage(curStage), 
	stats(data), 
	status(data),
	anim(aniMap, data), 
	movement(startPos, startingLane), 
	id(id)
{}

void Unit::destroy_unit(DeathCause deathCauue) {
	std::cout << "destroying Unit with id #" << id << std::endl;
	causeOfDeath |= deathCauue;

	movement.cancel_tween();

	// THe base being destroyed means all units should die unconditionally 
	// (no tranforming or reviving), and the death counter should not increment
	if (deathCauue != DeathCause::BASE_WAS_DESTROYED) {
		stage->recorder->add_death(stats->team, movement.currentLane, causeOfDeath);

		if (spawnCategory == UnitSpawnType::SUMMON)
			stage->lower_summons_count(stats->id);
		else if (status.can_transform()) {
			std::cout << "transforming" << std::endl;
			anim.start(UnitAnimationState::TRANSFORM);
			return;
		}
	}

	status.hp = 0;
	anim.die();
}
void Unit::call_death_anim(DeathCause deathCause) {
	causeOfDeath |= deathCause;
	anim.start(UnitAnimationState::DEATH);
}
bool Unit::move_req_check() { return !stage->can_push_move_request(id); }

float calc_phase_timer(float distance, float speed) {
	return (distance / speed);
}
void Unit::try_knockback(int oldHp, int enemyHitIndex, const UnitStats* enemyStats) {
	if (anim.in_knockback()) return;

	if (status.met_knockback_threshold(oldHp, stats)) {
		if (auto shield = stats->get_augment(AugmentType::SHIELD)) status.shieldHp = shield->intValue;
		float force = enemyStats->has_augment(AugmentType::BULLY) ? 1.5f : 1;

		if (enemyStats->has_augment(AugmentType::SQUASH))
			movement.push_squash_request(*this);
		else if (enemyStats->has_augment(AugmentType::LAUNCH))
			movement.push_launch_request(*this);
		else
			movement.knockback(*this, force);
	}
	else if (enemyStats->try_proc_augment(AugmentType::SHOVE, enemyHitIndex))
		movement.knockback(*this, 0.5f);
	else if (enemyStats->try_proc_augment(AugmentType::WARP, enemyHitIndex))
		movement.warp(*this, enemyStats);
}

// Checks
bool Unit::enemy_in_range(float xPos, float minRange, float maxRange) const {
	float dist = (xPos - get_pos().x) * (float)stats->team;
	return dist >= minRange && dist <= maxRange;
}
bool Unit::enemy_is_in_sight_range() const{
	float sightMultiplier = status.blinded() ? 0.5f : 1;
	float sightDist = stats->sightRange * sightMultiplier;

	// Check if the enemy's base is in range
	if (enemy_in_range(stage->get_enemy_base(stats->team).xPos(), 0, sightDist))
		return true;

	auto [minLane, maxLane] = get_lane_sight_range();
	
	for (int i = minLane; i < maxLane; i++) {
		std::vector<Unit>& enemyUnits = stage->lanes[i].getOpponentUnits(stats->team);

		for (auto it = enemyUnits.begin(); it != enemyUnits.end(); ++it) 
			if (found_valid_target(*it, 0, sightDist)) return true;
	}

	return false;
}
bool Unit::found_valid_target(const Unit& enemy, float minRange, float maxRange) const {
	return !enemy.anim.invincible() && enemy_in_range(enemy.movement.pos.x, minRange, maxRange);
}
bool Unit::over_gap() const { 
	// Check if the front of the hurtbox, as well as the back are BOTH off the lane
	const auto& [left, right] = getHurtboxEdges();
	return stage->lanes[movement.currentLane].within_gap(left, right);
}

bool Unit::can_make_surge(const Augment& aug) const {
	return aug.is_surge() && Random::chance(aug.percentage) &&
		aug.can_hit(combat.hitIndex);
}
bool Unit::rust_type_and_near_gap() const {
	if (!stats->rusted_tyoe()) return false;

	return std::ranges::any_of(stage->lanes[get_lane()].gaps, [&](const auto& gap) {
		float gapEdge = player_team() ? gap.first : gap.second; // choose between left and right edge
		float dist = (gapEdge - movement.pos.x) * static_cast<float>(stats->team);

		return dist > 0 && dist <= RUST_TYPE_LEDGE_RANGE;
	});
}

// Tick
void Unit::tick(float deltaTime) {
	if (status.overloaded()) deltaTime *= 0.5f;

	combat.cooldown -= deltaTime;
	status.update_status_effects(*this, deltaTime);

	switch (anim.get_state()) {
	case UnitAnimationState::MOVE:
		moving_state(deltaTime);
		break;
	case UnitAnimationState::ATTACK:
		attack_state(deltaTime);
		break;
	case UnitAnimationState::IDLE:
		idling_state(deltaTime);
		break;
	case UnitAnimationState::KNOCKBACK:
		knockback_state(deltaTime);
		break;
	case UnitAnimationState::FALLING:
		falling_state(deltaTime);
		break;
	case UnitAnimationState::JUMPING:
		jumping_state(deltaTime);
		break;
	case UnitAnimationState::PHASE_WINDUP:
		phase_windup_state(deltaTime);
		break;
	case UnitAnimationState::PHASE_ACTIVE:
		phase_active_state(deltaTime);
		break;
	case UnitAnimationState::PHASE_WINDDOWN:
		phase_winddown_state(deltaTime);
		break;
	case UnitAnimationState::TRANSFORM:
		transform_state(deltaTime);
		break;
	default:
		throw std::runtime_error("The Unit's current state has no");
		break;
	}
}

#pragma region Unit States
void Unit::moving_state(float deltaTime) {
	movement.move(*this, deltaTime);
	anim.update(deltaTime);
	anim.set_position(movement.pos);

	if (can_fall())
		movement.push_fall_request(*this);
	else if (enemy_is_in_sight_range()) {
		if (status.can_phase()) anim.start(UnitAnimationState::PHASE_WINDUP);
		else anim.start_idle_or_attack_animation(*this);
	}
	else {
		// in case Unit has both JUMP and LEAP, return if they succeed in jumping
		if (stats->has_augment(AugmentType::JUMP) && get_lane() < stage->laneCount - 1 && 
			movement.try_push_jump_request(*this)) 
			return; 
		if (stats->has_augment(AugmentType::LEAP) && movement.try_leap(*this)) return;
		if (rust_type_and_near_gap())
			anim.start(UnitAnimationState::IDLE);
	}
}
void Unit::attack_state(float deltaTime) {
	auto events = anim.update(deltaTime);

	if (any(events & AnimationEvent::FINAL_FRAME)) {
		combat.cooldown = stats->attackTime;
		if (stats->has_augment(AugmentType::FRAGILE)) destroy_unit();
		else if (enemy_is_in_sight_range()) 
			anim.start_idle_or_attack_animation(*this);	
		else if (!rust_type_and_near_gap())
			anim.start(UnitAnimationState::MOVE);
		else
			anim.start(UnitAnimationState::IDLE);
	}
	if (any(events & AnimationEvent::ATTACK)) {
		combat.attack(*this);
		if (stats->has_augment(AugmentType::FRAGILE)) status.hp = 0;
	}
}
void Unit::idling_state(float deltaTime) {
	anim.update(deltaTime);

	if (enemy_is_in_sight_range())
		anim.start_idle_or_attack_animation(*this);
	else if (!rust_type_and_near_gap())
		anim.start(UnitAnimationState::MOVE);
}
void Unit::knockback_state(float deltaTime) {
	anim.update(deltaTime);
	anim.set_position(movement.pos);

	if (can_fall()) movement.push_fall_request(*this);
	else if (!movement.tweening()) {
		if (status.dead()) anim.start(UnitAnimationState::DEATH);
		else anim.start_move_idle_or_attack(*this);
	}
	else {
		// If the tween ends, and it was a LAUNCH tween, enter the drop portion of it.
		UnitMoveRequestType finishedType = movement.update_tween(deltaTime);
		if (finishedType == UnitMoveRequestType::LAUNCH) movement.finish_launch_tween(stage);
	}
}
void Unit::falling_state(float deltaTime) {
	auto events = anim.update(deltaTime);
	anim.set_position(movement.pos);

	if (movement.tweening()) movement.update_tween(deltaTime);
	else if (any(events & AnimationEvent::FINAL_FRAME)) {
		if (status.dead()) destroy_unit(DeathCause::FALLING);
		else anim.start_move_idle_or_attack(*this);
	}
}
void Unit::jumping_state(float deltaTime) {
	anim.update(deltaTime);
	anim.set_position(movement.pos);

	if (!movement.tweening()) anim.start_move_idle_or_attack(*this);
	else movement.update_tween(deltaTime);
}
void Unit::phase_windup_state(float deltaTime) {
	auto events = anim.update(deltaTime);

	if (anim.onFirstFrame()) remove(status.statusFlags, AugmentType::PHASE);
	else if (any(events & AnimationEvent::TRIGGER)) {
		if (auto phaseAugment = stats->get_augment(AugmentType::PHASE)) {
			float newX = get_pos().x + phaseAugment->value * static_cast<float>(stats->team);
			newX = stage->lanes[movement.currentLane].get_stopping_point
			(newX, stats->sightRange, stats->team, getHurtboxEdges());
			float dist = std::abs(movement.pos.y - newX);

			movement.create_tween({ newX, movement.pos.y }, calc_phase_timer(dist, stats->speed), UnitMoveRequestType::PHASE);
			anim.enter_is_phasing_state();
		}
	}
}
void Unit::phase_active_state(float deltaTime) {
	anim.update(deltaTime);

	if (movement.tweening()) movement.update_tween(deltaTime);
	else anim.start(UnitAnimationState::PHASE_WINDDOWN);
}
void Unit::phase_winddown_state(float deltaTime) {
	auto events = anim.update(deltaTime);

	if (any(events & AnimationEvent::FINAL_FRAME)) anim.start_move_idle_or_attack(*this);
}
void Unit::death_state(float deltaTime) {
	auto events = anim.update(deltaTime);

	// This happens if the Unit has a SELF_DESTRUCT augment, 
	// AND it does explosion damage
	if (any(events & AnimationEvent::TRIGGER)) {
		if (auto selfDestruct = stats->get_augment(AugmentType::SELF_DESTRUCT))
			combat.self_destruct(*this, *selfDestruct);
	}
	else if (any(events & AnimationEvent::FINAL_FRAME)) destroy_unit();
}
void Unit::transform_state(float deltaTime) {
	auto events = anim.update(deltaTime);

	if (any(events & AnimationEvent::TRIGGER)) stage->transform_unit(*this);
	if (any(events & AnimationEvent::FINAL_FRAME)) {
		// This flag will be marked in Status Constructor, meaning that they will
		// transform into another unit on death
		if (has(status.statusFlags & AugmentType::TRANSFORM)) {
			remove(status.statusFlags, AugmentType::TRANSFORM); // remove the flag
			destroy_unit(); // recall destroy_unit(), this time it'll actually die
		}
		// The only way the function will reach this is if this point is if it
		// is what a PREVIOUS unit transformed INTO
		else anim.start_move_idle_or_attack(*this);
	}
}
#pragma endregion

std::pair<int, int> Unit::get_lane_reach() const {
	const auto& [minLane, maxLane] = stats->get_hit_stats(combat.hitIndex).laneReach;

	// minLane.might be negative, s do std::abs
	int min = std::max(get_lane() - std::abs(minLane), 0);
	int max = std::min(get_lane() + maxLane, stage->laneCount-1);

	return { min, max };
}
std::pair<int, int> Unit::get_lane_sight_range() const {
	int min = std::max(get_lane() - stats->laneSight.first, 0);
	int max = std::min(get_lane() + stats->laneSight.second, stage->laneCount-1);

	return { min, max };
}
std::pair<float, float> Unit::getHurtboxEdges() const {
	float edge = movement.pos.x - (stats->hurtBox.x * (float)stats->team);
	return std::minmax(movement.pos.x, edge);
}