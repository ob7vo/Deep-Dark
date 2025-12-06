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

RequestType UnitMovement::update_tween(float deltaTime) {
	tween.update(pos, deltaTime);
	return tweening() ? RequestType::NOT_DONE : tween.tweenType;
}
void UnitMovement::create_tween(sf::Vector2f endPos, float time,
	RequestType tweenType, bool overwrite) 
{
	if (tweening()) {
		if (overwrite) cancel_tween();
		else return;
	}

	std::cout << "created tween" << std::endl;
	tween.start(pos, endPos, time, tweenType);
}

#pragma region Movement
// HORIZONTAL
void UnitMovement::move(const Unit& unit, float deltaTime) {
	float speed = unit.status.slowed() ? 0.1f : unit.stats->speed;
	pos.x += speed * deltaTime * unit.get_dir();
}
void UnitMovement::knockback(Unit& unit, float force) {
	if (unit.stats->has_augment(LIGHTWEIGHT)) force *= 1.5f;
	if (unit.stats->has_augment(HEAVYWEIGHT)) force *= 0.7f;

	float newX = pos.x - (KNOCKBACK_FORCE * force * unit.get_dir());
	auto [minBound, maxBound] = unit.stage->lanes[currentLane].get_lane_boundaries();
	newX = std::clamp(newX, minBound, maxBound);

	sf::Vector2f newPos({ newX, unit.stage->lanes[currentLane].yPos });

	force = std::min(force, 1.2f);
	create_tween(newPos, KNOCKBACK_DURATION * force, RequestType::KNOCKBACK);
	unit.anim.start(UnitAnimationState::KNOCKBACK);
}
bool UnitMovement::try_leap(Unit& unit) {
	for (auto const [gapLeft, gapRight] : unit.stage->lanes[currentLane].gaps) {
		float leapRange = unit.stats->get_augment(LEAP).value;
		float edge = unit.player_team() ? gapLeft : gapRight;

		float dist = (edge - pos.x) * static_cast<float>(unit.stats->team);
		if (dist <= 0.f || dist > leapRange || over_this_gap({ gapLeft, gapRight }, pos.x)) continue;

		float landingSpot = edge + LEDGE_SNAP * static_cast<float>(unit.stats->team);
		sf::Vector2f newPos({ landingSpot, pos.y });

		create_tween(newPos, LEAP_DURATION, RequestType::LEAP);

		unit.anim.start(UnitAnimationState::JUMPING);
		return true;
	}

	return false;
}

// VERTICAL
void UnitMovement::fall(Unit& unit, float newY) {
	sf::Vector2f newPos({ pos.x, newY });

	create_tween(newPos, FALL_DURATION, RequestType::FALL);

	unit.anim.start(UnitAnimationState::FALLING);
}
void UnitMovement::squash(Unit& unit, float newY) {
	sf::Vector2f newPos({ pos.x, newY });

	create_tween(newPos, SQUASH_DURATION, RequestType::SQUASH);
	unit.anim.start(UnitAnimationState::KNOCKBACK);
}
void UnitMovement::launch(Unit& unit, float newY) {
	sf::Vector2f newPos({ pos.x, newY - LAUNCH_FORCE });

	create_tween(newPos, LAUNCHING_DURATION, RequestType::LAUNCH);

	unit.anim.start(UnitAnimationState::KNOCKBACK);
}

// EITHER
void UnitMovement::jump(Unit& unit, float newX) {
	sf::Vector2f newPos({ newX, unit.stage->lanes[currentLane].yPos });

	create_tween(newPos, JUMP_DURATION, RequestType::JUMP);

	unit.anim.start(UnitAnimationState::JUMPING);
}
void UnitMovement::warp(Unit& unit, const UnitStats* enemyStats) {
	Augment aug = enemyStats->get_augment(WARP);

	currentLane = std::clamp(currentLane + aug.intValue, 0, unit.stage->laneCount - 1);

	float newX = pos.x - (aug.value * (float)unit.stats->team);
	auto [minBound, maxBound] = unit.stage->lanes[currentLane].get_lane_boundaries();
	pos.x = std::clamp(newX, minBound, maxBound);

	unit.combat.cooldown = aug.value2;
}
#pragma endregion

#pragma region Pushing Move Requests
void UnitMovement::push_teleport_request(Unit& unit, const Teleporter& tp) {
	if (!unit.stage->can_push_move_request(unit.id)) return;

	unit.anim.start(UnitAnimationState::MOVE);

	// If the telportor leads to the same Lane, theen theres no need for a Move Request
	if (tp.connectedLane != currentLane)
		unit.stage->push_move_request(unit, tp.connectedLane, tp.xDestination, RequestType::TELEPORT);
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
		unit.stage->push_move_request(unit, newLane, yPos, RequestType::FALL);
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
		unit.stage->push_move_request(unit, newLane, laneYPos, RequestType::SQUASH);
}
void UnitMovement::push_launch_request(Unit& unit) {
	if (currentLane == unit.stage->laneCount - 1 || unit.stats->has_augment(HEAVYWEIGHT)) {
		knockback(unit, false);
		return;
	}

	int newLane = unit.stage->find_lane_to_knock_to(unit, 1);
	float laneYPos = unit.stage->lanes[newLane].yPos;

	// If there is no lane above, then default to a basic knockback
	if (newLane == currentLane) knockback(unit, false);
	else
		unit.stage->push_move_request(unit, newLane, laneYPos, RequestType::LAUNCH);
}
bool UnitMovement::try_push_jump_request(Unit& unit) const {
	int targetLane = currentLane + 1;
	const auto& [leftEdge, rightEdge] = unit.getHurtboxEdges();

	if (unit.stage->lanes[targetLane].out_of_lane(leftEdge, rightEdge)) 
		return false;

	float jumpRange = unit.stats->get_augment(JUMP).value;
	const Lane& tarLane = unit.stage->lanes[targetLane];

	for (const auto& [gapLeft, gapRight] : tarLane.gaps) {
		if (!over_this_gap({ gapLeft, gapRight }, pos.x)) continue; 
		// only jump to gaps you are directly under

		float edge = unit.player_team() ? gapRight : gapLeft;
		float dist = (edge - pos.x) * static_cast<float>(unit.stats->team);

		if (dist <= 0 || dist > jumpRange) return false;

		float landingSpot = edge + LEDGE_SNAP * static_cast<float>(unit.stats->team);
		unit.stage->push_move_request(unit, targetLane, landingSpot, RequestType::JUMP);

		return true;
	}
	return false;
}

void UnitMovement::finish_launch_tween(const Stage* stage) {
	float laneYPos = stage->lanes[currentLane].yPos;
	sf::Vector2f newPos = { pos.x, laneYPos };

	create_tween(newPos, DROPPING_DURATION, RequestType::DROP_FROM_LAUNCH);	
}
#pragma endregion

// Unit Tween Functions
void UnitTween::start(sf::Vector2f val, const sf::Vector2f& end, float dur, RequestType type) {
	startPos = val;
	endPos = end;
	duration = dur;
	tweenType = type;

	active = true;
}
void UnitTween::update(sf::Vector2f& unitPos, float deltaTime) {
	if (!active) return;
	elapsedTime += deltaTime;

	if (elapsedTime >= duration) {
		elapsedTime = duration;
		updateValue(unitPos);
		active = false;
	}
	else updateValue(unitPos);
}
sf::Vector2f UnitTween::getEasedT() const {
	float t = elapsedTime / duration;
	const auto& [easeX, easeY] = unitTweenEasings[static_cast<int>(tweenType)];

	float tX = easeFuncArr[static_cast<int>(easeX)](t);
	float tY = easeFuncArr[static_cast<int>(easeY)](t);

	return { tX, tY };
}
void UnitTween::updateValue(sf::Vector2f& unitPos) const {
	const auto& [easingFuncX, easingFuncY] = unitTweenEasings[static_cast<int>(tweenType)];

	sf::Vector2f t = getEasedT();
	unitPos = startPos + ((endPos - startPos) * t);

	/*
	if (easingFuncX < EasingType::COUNT) {
		float t = getEasedTX(easingFuncX);
		unitPos.x = startVec.x + (endVec.x - startVec.x) * t;
	}
	if (easingFuncY < EasingType::COUNT) {
		float t = getEasedTY(easingFuncY);
		unitPos.y = startVec.y + (endVec.y - startVec.y) * t;
	}
	*/
}
