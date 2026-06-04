#include "pch.h"
#include "UnitMovement.h"
#include "Unit.h"
#include "Stage.h"

using namespace UnitConfig;

void UnitMovement::setup(sf::Vector2f pos, int lane) {
	this->pos = pos;
	laneIdx = lane;
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
	newX = owner.stage->clamp_within_lane(newX, laneIdx);

	sf::Vector2f newPos({ newX, owner.stage->lanes[laneIdx].yPos });

	// The timer of knockback should change proportionally with the KB force
	float timerMultiplier = std::clamp(force, MIN_KB_TIMER_MULTIPLIER, MAX_KB_TIMER_MULTIPLIER);

	create_tween(newPos, KNOCKBACK_DURATION * timerMultiplier, UnitMoveRequestType::KNOCKBACK);
	owner.anim.start(UnitAnimationState::KNOCKBACK);
}
bool UnitMovement::try_leap() {
	float leapRange = owner.stats->get_augment(AugmentType::LEAP)->data.mobility.distance;

	for (auto const& [gapLeft, gapRight] : owner.stage->lanes[laneIdx].gaps) {
		float edge = owner.player_team() ? gapLeft : gapRight;

		if (float dist = (edge - pos.x) * static_cast<float>(owner.stats->team);
			dist <= 0.f || dist > leapRange || over_this_gap({ gapLeft, gapRight }, pos.x)) 
			continue;

		float landingSpot = edge + LEDGE_SNAP * static_cast<float>(owner.stats->team);
		sf::Vector2f newPos({ landingSpot, pos.y });

		create_tween(newPos, LEAP_TWEEN_DURATION, UnitMoveRequestType::LEAP);

		owner.anim.start(UnitAnimationState::JUMPING);
		return true;
	}

	return false;
}

// VERTICAL
void UnitMovement::fall(float newY, UnitMoveRequestType fallType) {
	sf::Vector2f newPos({ pos.x, newY });

	create_tween(newPos, FALL_DURATION, fallType);

	owner.anim.start(UnitAnimationState::FALLING);
}
void UnitMovement::squash(float newY) {
	sf::Vector2f newPos({ pos.x, newY });

	create_tween(newPos, SQUASH_DURATION, UnitMoveRequestType::SQUASH);
	owner.anim.start(UnitAnimationState::KNOCKBACK);
}
void UnitMovement::launch(float newY) {
	sf::Vector2f newPos({ pos.x, newY - LAUNCH_DISTANCE });

	create_tween(newPos, LAUNCH_TWEEN_DURATION, UnitMoveRequestType::LAUNCH);

	owner.anim.start(UnitAnimationState::KNOCKBACK);
}

// EITHER
void UnitMovement::jump(float newX) {
	sf::Vector2f newPos({ newX, owner.stage->lanes[laneIdx].yPos });

	create_tween(newPos, JUMP_TWEEN_DURATION, UnitMoveRequestType::JUMP);

	owner.anim.start(UnitAnimationState::JUMPING);
}
void UnitMovement::warp(float newY) {
	// Set the position
	pos.y = newY;

	// Get LaneYPos to check for falling
	float laneYpos = owner.stage->lanes[laneIdx].yPos;
	
	// Be assured that th only way for the Unit to be above or below laneYPos
	// Is if laneIdx = 0 and it below it, or laneIdx = MAX and is above it
	
	// Unit was warped above max Lane Index, at laneYPos - WARP_ABOVE_LANES_DISTANCE
	if (Screen::above(newY, laneYpos)) {
		// The Unit will fall back down to the lane, taking fall damage
		// however, if they are above gaps, they will fall throught those as well, taking additional damage.
		const auto [yPos, newLaneIdx] = stage->find_lane_to_fall_on(owner, laneIdx);

		float fallDmgPercent = WARP_BASE_FALL_DAMAGE_PERCENTAGE + (WARP_FALL_DAMAGE_PER_LANE * ((stage->laneCount-1) - laneIdx));
		owner.status.hp -= static_cast<int>(owner.stats->maxHp * fallDmgPercent);
		owner.status.hp = std::max(0, owner.status.hp);

		// If they land on a new lane, but not out of bounds
		if (newLaneIdx >= 0 && newLaneIdx < laneIdx) {
			stage->push_move_request(owner, newLaneIdx, yPos, UnitMoveRequestType::FALL);
		}
		else {
			if (newLaneIdx <= -1) owner.status.hp = 0;
			fall(yPos);
		}
	}
	// Unit spawned below min Lane Index, at laneYPos + WARP_UNDER_LANES_DISTANCE
	// Will fall out and die
	else if (Screen::below(newY, laneYpos)) {
		owner.status.hp = 0;
		fall(newY);
	}
	// It is at it's lane height, but may still fall through any gaps
	// No flal damage occurs in this case, but the Unit can still fall out and die
	else if (owner.over_gap()) {
		push_fall_request();
	}
}
#pragma endregion

#pragma region Pushing Move Requests
void UnitMovement::push_teleport_request(const Teleporter& tp) {
	if (!stage->can_push_move_request(owner.spawnID)) return;

	owner.anim.start(UnitAnimationState::MOVE);

	// If the telportor leads to the same Lane, theen theres no need for a Move Request
	if (tp.connectedLane != laneIdx)
		stage->push_move_request(owner, tp.connectedLane, tp.xDestination, UnitMoveRequestType::TELEPORT);
	else pos = { tp.xDestination, stage->lanes[tp.connectedLane].yPos };
}
void UnitMovement::push_fall_request() {
	const auto [yPos, newLaneIdx] = stage->find_lane_to_fall_on(owner, laneIdx - 1);

	// Lane will be -1 if no lane to fall on is found 
	// (aka, they fell through ALL lanes, straight into the void)

	if (newLaneIdx <= -1) {
		owner.status.hp = 0;
		fall(yPos, UnitMoveRequestType::FAST_FALL);
	}
	else
		stage->push_move_request(owner, newLaneIdx, yPos, UnitMoveRequestType::FALL);
}
void UnitMovement::push_squash_request() {
	if (laneIdx == 0) {
		knockback(false);
		return;
	}

	int newLane = stage->find_lane_to_knock_to(owner, -1);
	float laneYPos = stage->lanes[newLane].yPos;

	// If there is no lane below, then default to a basic knockback
	if (newLane == laneIdx) knockback(false);
	else
		stage->push_move_request(owner, newLane, laneYPos, UnitMoveRequestType::SQUASH);
}
void UnitMovement::push_launch_request() {
	if (laneIdx == owner.stage->laneCount - 1 ||
		owner.stats->has_augment(AugmentType::HEAVYWEIGHT)) {
		knockback(false);
		return;
	}

	int newLane = stage->find_lane_to_knock_to(owner, 1);
	float laneYPos = stage->lanes[newLane].yPos;

	// If there is no lane above, then default to a basic knockback
	if (newLane == laneIdx) knockback(false);
	else
		stage->push_move_request(owner, newLane, laneYPos, UnitMoveRequestType::LAUNCH);
}
void UnitMovement::push_warp_request(const Augment& warpAug) {
	// Reset animation to avoid any potential complications
	owner.anim.start(UnitAnimationState::WAITING);

	// The Unit's New Lane and x position are set BEFORE being sent into the warp waiting room
	// After it is done waiting, it will be sent to its newLane and decide its y position
	int actualLane = laneIdx + warpAug.data.warp.laneDisplacemnet;
	int clampedLane = std::clamp(actualLane, 0, stage->laneCount - 1);

	float newX = pos.x - (warpAug.data.warp.distance  * (float)owner.stats->team);
	pos.x = stage->clamp_within_lane(newX, clampedLane);

	float newY = stage->lanes[clampedLane].yPos;
	if (actualLane < -1)
		newY += WARP_UNDER_LANES_DISTANCE;
	else if (actualLane >= stage->laneCount)
		newY -= WARP_ABOVE_LANES_DISTANCE;

	stage->push_move_request(owner, clampedLane, newY, UnitMoveRequestType::WARP);
}
bool UnitMovement::try_push_jump_request() const {
	int targetLane = laneIdx + 1;
	const auto& [leftEdge, rightEdge] = owner.getHurtboxEdges();

	if (stage->lanes[targetLane].out_of_lane(leftEdge, rightEdge))
		return false;

	float jumpRange = owner.stats->get_augment(AugmentType::JUMP)->data.mobility.distance;

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
	float laneYPos = stage->lanes[laneIdx].yPos;
	sf::Vector2f newPos = { pos.x, laneYPos };

	create_tween(newPos, DROP_FROM_LAUNCH_TWEEN_DURATION, UnitMoveRequestType::DROP_FROM_LAUNCH);	
}
#pragma endregion