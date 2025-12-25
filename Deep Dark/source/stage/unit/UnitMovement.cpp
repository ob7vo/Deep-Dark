#include "pch.h"
#include "UnitMovement.h"
#include "Unit.h"
#include "Stage.h"

const float KNOCKBACK_FORCE = 50.0f;
const float KNOCKBACK_DURATION = 1.4f;
const float FALL_DURATION = 1.25f;
const float SQUASH_DURATION = 1.3f;
const float LAUNCHING_DURATION = .65f;
const float DROPPING_DURATION = .7f;
const float LAUNCH_FORCE = 75.f;

const float LEDGE_SNAP = 25.0f;
const float JUMP_DURATION = 1.f;
const float LEAP_DURATION = 0.7f;

UnitMovement::UnitMovement(sf::Vector2f pos, int lane) : currentLane(lane), pos(pos) {}


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
void UnitMovement::knockback(Unit& unit, float force) {
	if (unit.stats->has_augment(AugmentType::LIGHTWEIGHT)) force *= 1.5f;
	if (unit.stats->has_augment(AugmentType::HEAVYWEIGHT)) force *= 0.7f;

	float newX = pos.x - (KNOCKBACK_FORCE * force * unit.get_dir());
	newX = unit.stage->clamp_within_lane(newX, currentLane);

	sf::Vector2f newPos({ newX, unit.stage->lanes[currentLane].yPos });

	force = std::min(force, 1.2f);
	create_tween(newPos, KNOCKBACK_DURATION * force, UnitMoveRequestType::KNOCKBACK);
	unit.anim.start(UnitAnimationState::KNOCKBACK);
}
bool UnitMovement::try_leap(Unit& unit) {
	float leapRange = unit.stats->get_augment(AugmentType::LEAP)->value;

	for (auto const [gapLeft, gapRight] : unit.stage->lanes[currentLane].gaps) {
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
	sf::Vector2f newPos({ pos.x, newY - LAUNCH_FORCE });

	create_tween(newPos, LAUNCHING_DURATION, UnitMoveRequestType::LAUNCH);

	unit.anim.start(UnitAnimationState::KNOCKBACK);
}

// EITHER
void UnitMovement::jump(Unit& unit, float newX) {
	sf::Vector2f newPos({ newX, unit.stage->lanes[currentLane].yPos });

	create_tween(newPos, JUMP_DURATION, UnitMoveRequestType::JUMP);

	unit.anim.start(UnitAnimationState::JUMPING);
}
void UnitMovement::warp(Unit& unit, const UnitStats* enemyStats) {
	const Augment& warpAug = *enemyStats->get_augment(AugmentType::WARP);

	currentLane = std::clamp(currentLane + warpAug.intValue, 0, unit.stage->laneCount - 1);

	float newX = pos.x - (warpAug.value * (float)unit.stats->team);
	pos.x = unit.stage->clamp_within_lane(newX, currentLane);

	unit.combat.cooldown = warpAug.value2;
}
#pragma endregion

#pragma region Pushing Move Requests
void UnitMovement::push_teleport_request(Unit& unit, const Teleporter& tp) {
	if (!unit.stage->can_push_move_request(unit.id)) return;

	unit.anim.start(UnitAnimationState::MOVE);

	// If the telportor leads to the same Lane, theen theres no need for a Move Request
	if (tp.connectedLane != currentLane)
		unit.stage->push_move_request(unit, tp.connectedLane, tp.xDestination, UnitMoveRequestType::TELEPORT);
	else pos = { tp.xDestination, unit.stage->lanes[tp.connectedLane].yPos };
}
void UnitMovement::push_fall_request(Unit& unit) {
	const auto [yPos, newLane] = unit.stage->find_lane_to_fall_on(unit);

	// Lane will be -1 if no lane to fall on is found 
	// (aka, they fell through ALL lanes, straight into the void)

	if (newLane <= -1) {
		unit.status.hp = 0;
		fall(unit, yPos);
	}
	else
		unit.stage->push_move_request(unit, newLane, yPos, UnitMoveRequestType::FALL);
}
void UnitMovement::push_squash_request(Unit& unit) {
	if (currentLane == 0) {
		knockback(unit, false);
		return;
	}

	int newLane = unit.stage->find_lane_to_knock_to(unit, -1);
	float laneYPos = unit.stage->lanes[newLane].yPos;

	// If there is no lane below, then default to a basic knockback
	if (newLane == currentLane) knockback(unit, false);
	else
		unit.stage->push_move_request(unit, newLane, laneYPos, UnitMoveRequestType::SQUASH);
}
void UnitMovement::push_launch_request(Unit& unit) {
	if (currentLane == unit.stage->laneCount - 1 || 
		unit.stats->has_augment(AugmentType::HEAVYWEIGHT)) {
		knockback(unit, false);
		return;
	}

	int newLane = unit.stage->find_lane_to_knock_to(unit, 1);
	float laneYPos = unit.stage->lanes[newLane].yPos;

	// If there is no lane above, then default to a basic knockback
	if (newLane == currentLane) knockback(unit, false);
	else
		unit.stage->push_move_request(unit, newLane, laneYPos, UnitMoveRequestType::LAUNCH);
}
bool UnitMovement::try_push_jump_request(Unit& unit) const {
	int targetLane = currentLane + 1;
	const auto& [leftEdge, rightEdge] = unit.getHurtboxEdges();

	if (unit.stage->lanes[targetLane].out_of_lane(leftEdge, rightEdge)) 
		return false;

	float jumpRange = unit.stats->get_augment(AugmentType::JUMP)->value;

	for (const auto& [gapLeft, gapRight] : unit.stage->lanes[targetLane].gaps) {
		if (!over_this_gap({ gapLeft, gapRight }, pos.x)) continue; 
		// only jump to gaps you are directly under

		float edge = unit.player_team() ? gapRight : gapLeft;

		if (float dist = (edge - pos.x) * static_cast<float>(unit.stats->team); 
			dist <= 0 || dist > jumpRange) return false;

		float landingSpot = edge + LEDGE_SNAP * static_cast<float>(unit.stats->team);
		unit.stage->push_move_request(unit, targetLane, landingSpot, UnitMoveRequestType::JUMP);

		return true;
	}

	return false;
}

void UnitMovement::finish_launch_tween(const Stage* stage) {
	float laneYPos = stage->lanes[currentLane].yPos;
	sf::Vector2f newPos = { pos.x, laneYPos };

	create_tween(newPos, DROPPING_DURATION, UnitMoveRequestType::DROP_FROM_LAUNCH);	
}
#pragma endregion