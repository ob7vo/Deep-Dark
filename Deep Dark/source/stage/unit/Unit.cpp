#include "pch.h"
#include "Unit.h"
#include "Stage.h"
#include "Utils.h"
#include "Surge.h"
#include "StageRecord.h"

const float RUST_TYPE_LEDGE_RANGE = 15.f;
const float BASE_PHASING_TIMER = 1.f; // over 50 distance
const float BASE_PHASE_DISTANCE = 50;

Unit::Unit(Stage* curStage, sf::Vector2f startPos, int startingLane, const UnitStats* data,
	UnitAniMap* aniMap, int id)
	: stage(curStage),anim(aniMap), movement(startPos, startingLane), stats(data), id(id), status(data)
{}


void Unit::destroy_unit() {
	std::cout << "destroying Unit with id #" << id << std::endl;
	
	movement.cancel_tween();
	stage->recorder->add_death(stats->team, movement.currentLane, causeOfDeath);
	if (spawnCategory == SpawnCategory::SUMMON) stage->summonData->count--;

	status.hp = 0;
	anim.die();
}
bool Unit::move_req_check() { return !stage->can_push_move_request(id); }


float calculate_phase_timer(float distance) {
	return (distance / BASE_PHASE_DISTANCE) * BASE_PHASING_TIMER;
}

// Drawing and Animating

// Movement

void Unit::try_knockback(int oldHp, int enemyHitIndex, const UnitStats* enemyStats) {
	if (anim.in_knockback()) return;
	if (status.met_knockback_threshold(oldHp, stats)) {
		status.shieldHp = stats->get_augment(SHIELD).intValue;
		float force = enemyStats->has_augment(BULLY) ? 1.5f : 1;

		if (enemyStats->has_augment(AugmentType::SQUASH))
			movement.push_squash_request(*this);
		else if (enemyStats->has_augment(AugmentType::LAUNCH))
			movement.push_launch_request(*this);
		else
			movement.knockback(*this, force);
	}
	else if (enemyStats->try_proc_augment(SHOVE, enemyHitIndex))
		movement.knockback(*this, 0.5f);
	else if (enemyStats->try_proc_augment(WARP, enemyHitIndex))
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
	if (enemy_in_range(stage->get_enemy_base(stats->team).pos.x, 0, sightDist))
		return true;

	auto [minLane, maxLane] = get_lane_sight_range();
	
	for (int i = minLane; i < maxLane; i++) {
		std::vector<Unit>& enemyUnits = stage->get_lane_targets(i, stats->team);
		for (auto it = enemyUnits.begin(); it != enemyUnits.end(); ++it) {
			if (found_valid_target(*it, 0, sightDist)) return true;
		}
	}

	return false;
}
bool Unit::found_valid_target(const Unit& enemy, float minRange, float maxRange) const {
	return !enemy.anim.invincible() && enemy_in_range(enemy.movement.pos.x, minRange, maxRange);
}
bool Unit::over_gap() const { return stage->over_gap(get_lane(), get_pos().x); }

bool Unit::can_make_surge(const Augment& aug) const {
	return aug.is_surge() && Random::chance(aug.percentage) &&
		aug.can_hit(combat.hitIndex);
}
bool Unit::rust_type_and_near_gap() const {
	if (!stats->rusted_tyoe()) return false;

	for (auto const [leftEdge, rightEdge] : stage->get_lane(get_lane()).gaps) {
		float edge = player_team() ? leftEdge : rightEdge;
		float dist = (edge - get_pos().x) * static_cast<float>(stats->team);

		if (dist > 0 && dist <= RUST_TYPE_LEDGE_RANGE)
			return true;
	}

	return false;
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
	case UnitAnimationState::PHASE:
		phase_state(deltaTime);
		break;
	default:
		waiting_state();
		break;
	}
}

#pragma region Unit States
void Unit::moving_state(float deltaTime) {
	anim.update(deltaTime);
	anim.set_position(movement.pos);
	movement.move(*this, deltaTime);

	if (can_fall())
		movement.push_fall_request(*this);
	else if (enemy_is_in_sight_range()) {
		if (status.can_phase()) anim.start(UnitAnimationState::PHASE);
		else anim.start_idle_or_attack_animation(*this);
	}
	else {
		// in case Unit has both JUMP and LEAP, return if they succeed in jumping
		if (has_augment(JUMP) && get_lane() < stage->laneCount - 1 && movement.try_push_jump_request(*this)) 
			return; 
		if (has_augment(LEAP) && movement.try_leap(*this)) return;
		if (rust_type_and_near_gap())
			anim.start(UnitAnimationState::IDLE);
	}
}
void Unit::attack_state(float deltaTime) {
	int events = anim.update(deltaTime);

	if (events & FINAL_FRAME) {
		combat.cooldown = stats->attackTime;
		if (has_augment(SELF_DESTRUCT)) destroy_unit();
		else if (enemy_is_in_sight_range()) 
			anim.start_idle_or_attack_animation(*this);	
		else if (!rust_type_and_near_gap())
			anim.start(UnitAnimationState::MOVE);
		else
			anim.start(UnitAnimationState::IDLE);
	}
	if (events & ATTACK) {
		combat.attack(*this);
		if (has_augment(SELF_DESTRUCT)) status.hp = 0;
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

	if (can_fall()) 
		movement.push_fall_request(*this);
	else if (!movement.tweening()) {
		if (status.dead()) {
			//std:: << "died after knockback" << std::endl;
			destroy_unit();
		}
		else if (enemy_is_in_sight_range())
			anim.start_idle_or_attack_animation(*this);
		else anim.start(UnitAnimationState::MOVE);
	}
	else {
		RequestType finishedType = movement.update_tween(deltaTime);
		if (finishedType == RequestType::LAUNCH) movement.finish_launch_tween(*this);
	}
}
void Unit::falling_state(float deltaTime) {
	int events = anim.update(deltaTime);
	anim.set_position(movement.pos);

	if (movement.tweening()) movement.update_tween(deltaTime);
	else if (events & FINAL_FRAME) {
		if (status.dead()) {
			//std:: << "died after falling" << std::endl;
			destroy_unit();
		}
		else if (enemy_is_in_sight_range())
			anim.start_idle_or_attack_animation(*this);
		else anim.start(UnitAnimationState::MOVE);
	}
}
void Unit::jumping_state(float deltaTime) {
	anim.update(deltaTime);
	anim.set_position(movement.pos);

	if (!movement.tweening()) {
		if (status.dead()) destroy_unit();
		else if (enemy_is_in_sight_range())
			anim.start_idle_or_attack_animation(*this);
		else anim.start(UnitAnimationState::MOVE);
	}
	else movement.update_tween(deltaTime);
}
void Unit::phase_state(float deltaTime) {
	auto events = anim.update(deltaTime);

	if (Animation::check_for_event(AnimationEvent::FINAL_FRAME, events)) {
		if (status.can_phase()) { // if can still phase
			status.statusFlags &= ~PHASE;

			float end = get_pos().x + stats->get_augment(PHASE).value * static_cast<float>(stats->team);
			auto [minWall, maxWall] = stage->get_walls(get_lane());
			end = std::clamp(end, minWall, maxWall);

			float timer = calculate_phase_timer(std::abs(get_pos().x - end));

			movement.create_tween({ end, get_pos().y}, timer, RequestType::PHASE);
			combat.cooldown = timer;
			anim.start_phasing();
		}
		else {
			if (status.dead()) destroy_unit();
			else if (enemy_is_in_sight_range())
				anim.start_idle_or_attack_animation(*this);
			else anim.start(UnitAnimationState::MOVE);
		}
	}
}
void Unit::waiting_state() {
	if (combat.cooldown <= 0) {
		if (anim.is_phasing())
			anim.start(UnitAnimationState::PHASE);
		else {
			if (enemy_is_in_sight_range())
				anim.start_idle_or_attack_animation(*this);
			else
				anim.start(UnitAnimationState::MOVE);
		}
	}
}
#pragma endregion

std::pair<int, int> Unit::get_lane_reach() const {
	auto& hit = stats->get_hit_stats(combat.hitIndex);
	// laneReach.first will be negative
	int min = std::max(get_lane() - std::abs(hit.laneReach.first), 0);
	int max = std::min(get_lane() + hit.laneReach.second + 1, stage->laneCount);
	return { min, max };
}
std::pair<int, int> Unit::get_lane_sight_range() const {
	int min = std::max(get_lane() - stats->laneSight.first, 0);
	int max = std::min(get_lane() + stats->laneSight.second + 1, stage->laneCount);
	return { min, max };
}