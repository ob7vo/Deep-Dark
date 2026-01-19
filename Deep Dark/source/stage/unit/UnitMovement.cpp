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
void UnitMovement::move(const Unit& unit, float deltaTime) {
	float speed = unit.status.slowed() ? 0.1f : unit.stats->speed;
	pos.x += speed * deltaTime * unit.get_dir();
}
void UnitMovement::knockback(const Stage* stage, Unit& hitUnit, float force) {
	if (hitUnit.stats->has_augment(AugmentType::LIGHTWEIGHT)) force *= LIGHTWEIGHT_KB_FORCE;
	if (hitUnit.stats->has_augment(AugmentType::HEAVYWEIGHT)) force *= HEAVYWEIGHT_KB_FORCE;

	float newX = pos.x - (KNOCKBACK_DISTANCE * force * hitUnit.get_dir());
	newX = stage->clamp_within_lane(newX, laneInd);

	sf::Vector2f newPos({ newX, stage->lanes[laneInd].yPos });

	// The timer of knockback should change proportionally with the KB force
	float timerMultiplier = std::clamp(force, MIN_KB_TIMER_MULTIPLIER, MAX_KB_TIMER_MULTIPLIER);

	create_tween(newPos, KNOCKBACK_DURATION * timerMultiplier, UnitMoveRequestType::KNOCKBACK);
	hitUnit.anim.start(UnitAnimationState::KNOCKBACK);
}
bool UnitMovement::try_leap(const Stage* stage, Unit& unit) {
	float leapRange = unit.stats->get_augment(AugmentType::LEAP)->value;

	for (auto const& [gapLeft, gapRight] : stage->lanes[laneInd].gaps) {
		float edge = unit.player_team() ? gapLeft : gapRight;

		if (float dist = (edge - pos.x) * static_cast<float>(unit.stats->team);
			dist <= 0.f || dist > leapRange || over_this_gap({ gapLeft, gapRight }, pos.x)) 
			continue;

		float landingSpot = edge + LEDGE_SNAP * static_cast<float>(unit.stats->team);
		sf::Vector2f newPos({ landingSpot, pos.y });

		create_tween(newPos, LEAP_DURATION, UnitMoveRequestType::LEAP);

		unit.anim.start(UnitAnimationState::JUMPING);
		return true;
	}

	return false;
}

// VERTICAL
void UnitMovement::fall(Unit& unit, float newY) {
	sf::Vector2f newPos({ pos.x, newY });

	create_tween(newPos, FALL_DURATION, UnitMoveRequestType::FALL);

	unit.anim.start(UnitAnimationState::FALLING);
}
void UnitMovement::squash(Unit& unit, float newY) {
	sf::Vector2f newPos({ pos.x, newY });

	create_tween(newPos, SQUASH_DURATION, UnitMoveRequestType::SQUASH);
	unit.anim.start(UnitAnimationState::KNOCKBACK);
}
void UnitMovement::launch(Unit& unit, float newY) {
	sf::Vector2f newPos({ pos.x, newY - LAUNCH_DISTANCE });

	create_tween(newPos, LAUNCH_DURATION, UnitMoveRequestType::LAUNCH);

	unit.anim.start(UnitAnimationState::KNOCKBACK);
}

// EITHER
void UnitMovement::jump(const Stage* stage, Unit& unit, float newX) {
	sf::Vector2f newPos({ newX, unit.stage->lanes[laneInd].yPos });

	create_tween(newPos, JUMP_DURATION, UnitMoveRequestType::JUMP);

	unit.anim.start(UnitAnimationState::JUMPING);
}
void UnitMovement::warp(const Stage* stage, Unit& unit, const UnitStats* enemyStats) {
	const Augment& warpAug = *enemyStats->get_augment(AugmentType::WARP);

	laneInd = std::clamp(laneInd + warpAug.intValue, 0, stage->laneCount - 1);

	float newX = pos.x - (warpAug.value * (float)unit.stats->team);
	pos.x = stage->clamp_within_lane(newX, laneInd);

	unit.combat.cooldown = warpAug.value2;
}
#pragma endregion

#pragma region Pushing Move Requests
void UnitMovement::push_teleport_request(Stage* stage, Unit& unit, const Teleporter& tp) {
	if (!stage->can_push_move_request(unit.spawnID)) return;

	unit.anim.start(UnitAnimationState::MOVE);

	// If the telportor leads to the same Lane, theen theres no need for a Move Request
	if (tp.connectedLane != laneInd)
		stage->push_move_request(unit, tp.connectedLane, tp.xDestination, UnitMoveRequestType::TELEPORT);
	else pos = { tp.xDestination, stage->lanes[tp.connectedLane].yPos };
}
void UnitMovement::push_fall_request(Stage* stage, Unit& unit) {
	const auto [yPos, newLane] = stage->find_lane_to_fall_on(unit);

	// Lane will be -1 if no lane to fall on is found 
	// (aka, they fell through ALL lanes, straight into the void)

	if (newLane <= -1) {
		unit.status.hp = 0;
		fall(unit, yPos);
	}
	else
		stage->push_move_request(unit, newLane, yPos, UnitMoveRequestType::FALL);
}
void UnitMovement::push_squash_request(Stage* stage, Unit& unit) {
	if (laneInd == 0) {
		knockback(stage, unit, false);
		return;
	}

	int newLane = stage->find_lane_to_knock_to(unit, -1);
	float laneYPos = stage->lanes[newLane].yPos;

	// If there is no lane below, then default to a basic knockback
	if (newLane == laneInd) knockback(stage, unit, false);
	else
		stage->push_move_request(unit, newLane, laneYPos, UnitMoveRequestType::SQUASH);
}
void UnitMovement::push_launch_request(Stage* stage, Unit& unit) {
	if (laneInd == stage->laneCount - 1 || 
		unit.stats->has_augment(AugmentType::HEAVYWEIGHT)) {
		knockback(stage, unit, false);
		return;
	}

	int newLane = stage->find_lane_to_knock_to(unit, 1);
	float laneYPos = stage->lanes[newLane].yPos;

	// If there is no lane above, then default to a basic knockback
	if (newLane == laneInd) knockback(stage, unit, false);
	else
		stage->push_move_request(unit, newLane, laneYPos, UnitMoveRequestType::LAUNCH);
}
bool UnitMovement::try_push_jump_request(Stage* stage, Unit& unit) const {
	int targetLane = laneInd + 1;
	const auto& [leftEdge, rightEdge] = unit.getHurtboxEdges();

	if (stage->lanes[targetLane].out_of_lane(leftEdge, rightEdge)) 
		return false;

	float jumpRange = unit.stats->get_augment(AugmentType::JUMP)->value;

	for (const auto& [gapLeft, gapRight] : stage->lanes[targetLane].gaps) {
		if (!over_this_gap({ gapLeft, gapRight }, pos.x)) continue; 
		// only jump to gaps you are directly under

		float edge = unit.player_team() ? gapRight : gapLeft;

		if (float dist = (edge - pos.x) * static_cast<float>(unit.stats->team); 
			dist <= 0 || dist > jumpRange) return false;

		float landingSpot = edge + LEDGE_SNAP * static_cast<float>(unit.stats->team);
		stage->push_move_request(unit, targetLane, landingSpot, UnitMoveRequestType::JUMP);

		return true;
	}

	return false;
}

void UnitMovement::finish_launch_tween(const Stage* stage) {
	float laneYPos = stage->lanes[laneInd].yPos;
	sf::Vector2f newPos = { pos.x, laneYPos };

	create_tween(newPos, DROP_FROM_LAUNCH_DURATION, UnitMoveRequestType::DROP_FROM_LAUNCH);	
}
#pragma endregion