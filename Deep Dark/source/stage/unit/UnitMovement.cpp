#include "pch.h"
#include "UnitMovement.h"
#include "Unit.h"
#include "Stage.h"

using namespace UnitConfig;

void UnitMovement::setup(sf::Vector2f pos, int lane) {
	this->pos = pos;
	laneInd = lane;
}

bool over_this_gap(std::pair<float, float> gap, float xPos) {
	return xPos > gap.first && xPos < gap.second;
}

UnitMoveRequestType UnitMovement::update_tween(float deltaTime) {
	tween.update(pos, deltaTime);	

	return tweening() ? UnitMoveRequestType::NOT_DONE : tween.tweenType;
}
void UnitMovement::create_tween(sf::Vector2f endPos, float time,
	UnitMoveRequestType tweenType, bool overwrite) 
{
	if (tweening()) {
		if (overwrite) cancel_tween();
		else return;
	}

	tween.start(pos, endPos, time, tweenType);
}

#pragma region Movement
// HORIZONTAL
void UnitMovement::move(float deltaTime) {
	float speed = owner.status.is_slowed() ? 0.1f : owner.stats->speed;
	pos.x += speed * deltaTime * owner.stats->team;
}
void UnitMovement::knockback(float force) {
	if (owner.stats->has_augment(AugmentType::LIGHTWEIGHT)) force *= LIGHTWEIGHT_KB_FORCE;
	if (owner.stats->has_augment(AugmentType::HEAVYWEIGHT)) force *= HEAVYWEIGHT_KB_FORCE;

	float newX = pos.x - (KNOCKBACK_DISTANCE * force * owner.stats->team);
	newX = owner.stage->clamp_within_lane(newX, laneInd);

	sf::Vector2f newPos({ newX, owner.stage->lanes[laneInd].yPos });

	// The timer of knockback should change proportionally with the KB force
	float timerMultiplier = std::clamp(force, MIN_KB_TIMER_MULTIPLIER, MAX_KB_TIMER_MULTIPLIER);

	create_tween(newPos, KNOCKBACK_DURATION * timerMultiplier, UnitMoveRequestType::KNOCKBACK);
	owner.anim.start(UnitAnimationState::KNOCKBACK);
}
bool UnitMovement::try_leap() {
	float leapRange = owner.stats->get_augment(AugmentType::LEAP)->value;

	for (auto const& [gapLeft, gapRight] : owner.stage->lanes[laneInd].gaps) {
		float edge = owner.player_team() ? gapLeft : gapRight;

		if (float dist = (edge - pos.x) * static_cast<float>(owner.stats->team);
			dist <= 0.f || dist > leapRange || over_this_gap({ gapLeft, gapRight }, pos.x)) 
			continue;

		float landingSpot = edge + LEDGE_SNAP * static_cast<float>(owner.stats->team);
		sf::Vector2f newPos({ landingSpot, pos.y });

		create_tween(newPos, LEAP_DURATION, UnitMoveRequestType::LEAP);

		owner.anim.start(UnitAnimationState::JUMPING);
		return true;
	}

	return false;
}

// VERTICAL
void UnitMovement::fall(float newY) {
	sf::Vector2f newPos({ pos.x, newY });

	create_tween(newPos, FALL_DURATION, UnitMoveRequestType::FALL);

	owner.anim.start(UnitAnimationState::FALLING);
}
void UnitMovement::squash(float newY) {
	sf::Vector2f newPos({ pos.x, newY });

	create_tween(newPos, SQUASH_DURATION, UnitMoveRequestType::SQUASH);
	owner.anim.start(UnitAnimationState::KNOCKBACK);
}
void UnitMovement::launch(float newY) {
	sf::Vector2f newPos({ pos.x, newY - LAUNCH_DISTANCE });

	create_tween(newPos, LAUNCH_DURATION, UnitMoveRequestType::LAUNCH);

	owner.anim.start(UnitAnimationState::KNOCKBACK);
}

// EITHER
void UnitMovement::jump(float newX) {
	sf::Vector2f newPos({ newX, owner.stage->lanes[laneInd].yPos });

	create_tween(newPos, JUMP_DURATION, UnitMoveRequestType::JUMP);

	owner.anim.start(UnitAnimationState::JUMPING);
}
void UnitMovement::warp(const UnitStats* enemyStats) {
	const Augment& warpAug = *enemyStats->get_augment(AugmentType::WARP);

	laneInd = std::clamp(laneInd + warpAug.intValue, 0, stage->laneCount - 1);

	float newX = pos.x - (warpAug.value * (float)owner.stats->team);
	pos.x = stage->clamp_within_lane(newX, laneInd);

	owner.combat.cooldown = warpAug.value2;
}
#pragma endregion

#pragma region Pushing Move Requests
void UnitMovement::push_teleport_request(const Teleporter& tp) {
	if (!stage->can_push_move_request(owner.spawnID)) return;

	owner.anim.start(UnitAnimationState::MOVE);

	// If the telportor leads to the same Lane, theen theres no need for a Move Request
	if (tp.connectedLane != laneInd)
		stage->push_move_request(owner, tp.connectedLane, tp.xDestination, UnitMoveRequestType::TELEPORT);
	else pos = { tp.xDestination, stage->lanes[tp.connectedLane].yPos };
}
void UnitMovement::push_fall_request() {
	const auto [yPos, newLane] = stage->find_lane_to_fall_on(owner);

	// Lane will be -1 if no lane to fall on is found 
	// (aka, they fell through ALL lanes, straight into the void)

	if (newLane <= -1) {
		owner.status.hp = 0;
		fall(yPos);
	}
	else
		stage->push_move_request(owner, newLane, yPos, UnitMoveRequestType::FALL);
}
void UnitMovement::push_squash_request() {
	if (laneInd == 0) {
		knockback(false);
		return;
	}

	int newLane = stage->find_lane_to_knock_to(owner, -1);
	float laneYPos = stage->lanes[newLane].yPos;

	// If there is no lane below, then default to a basic knockback
	if (newLane == laneInd) knockback(false);
	else
		stage->push_move_request(owner, newLane, laneYPos, UnitMoveRequestType::SQUASH);
}
void UnitMovement::push_launch_request() {
	if (laneInd == owner.stage->laneCount - 1 ||
		owner.stats->has_augment(AugmentType::HEAVYWEIGHT)) {
		knockback(false);
		return;
	}

	int newLane = stage->find_lane_to_knock_to(owner, 1);
	float laneYPos = stage->lanes[newLane].yPos;

	// If there is no lane above, then default to a basic knockback
	if (newLane == laneInd) knockback(false);
	else
		stage->push_move_request(owner, newLane, laneYPos, UnitMoveRequestType::LAUNCH);
}
bool UnitMovement::try_push_jump_request() const {
	int targetLane = laneInd + 1;
	const auto& [leftEdge, rightEdge] = owner.getHurtboxEdges();

	if (stage->lanes[targetLane].out_of_lane(leftEdge, rightEdge))
		return false;

	float jumpRange = owner.stats->get_augment(AugmentType::JUMP)->value;

	for (const auto& [gapLeft, gapRight] : stage->lanes[targetLane].gaps) {
		if (!over_this_gap({ gapLeft, gapRight }, pos.x)) continue; 
		// only jump to gaps you are directly under

		float edge = owner.player_team() ? gapRight : gapLeft;

		if (float dist = (edge - pos.x) * static_cast<float>(owner.stats->team);
			dist <= 0 || dist > jumpRange) return false;

		float landingSpot = edge + LEDGE_SNAP * static_cast<float>(owner.stats->team);
		stage->push_move_request(owner, targetLane, landingSpot, UnitMoveRequestType::JUMP);

		return true;
	}

	return false;
}

void UnitMovement::finish_launch_tween() {
	float laneYPos = stage->lanes[laneInd].yPos;
	sf::Vector2f newPos = { pos.x, laneYPos };

	create_tween(newPos, DROP_FROM_LAUNCH_DURATION, UnitMoveRequestType::DROP_FROM_LAUNCH);	
}
#pragma endregion