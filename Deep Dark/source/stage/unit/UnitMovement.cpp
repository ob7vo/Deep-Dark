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

UnitMovement::UnitMovement(sf::Vector2f pos, int lane) : pos(pos), currentLane(lane) {}


bool over_this_gap(std::pair<float, float> gap, float xPos) {
	return xPos > gap.first && xPos < gap.second;
}

RequestType UnitMovement::update_tween(float deltaTime) {
	pos = tween->update_and_get(deltaTime);

	if (tween->isComplete) {
		RequestType type = tween->tweenType;
		tween = nullptr;

		return type;
	}

	return RequestType::NOT_DONE;
}
void UnitMovement::create_tween(sf::Vector2f endPos, float time,
	RequestType tweenType, bool overwrite) 
{
	if (tweening()) {
		if (overwrite) tween = nullptr;
		else return;
	}

	std::cout << "created tween" << std::endl;
	tween = std::make_unique<UnitTween>(pos, endPos, time, tweenType);
}

#pragma region Movement
// HORIZONTAL
void UnitMovement::move(Unit& unit, float deltaTime) {
	float speed = unit.status.slowed() ? 0.1f : unit.stats->speed;
	pos.x += speed * deltaTime * unit.get_dir();
}
void UnitMovement::knockback(Unit& unit, float force) {
	if (unit.stats->has_augment(LIGHTWEIGHT)) force *= 1.5f;
	if (unit.stats->has_augment(HEAVYWEIGHT)) force *= 0.7f;

	float newX = pos.x - (KNOCKBACK_FORCE * force * unit.get_dir());
	auto [minWall, maxWall] = unit.stage->get_walls(currentLane);
	newX = std::clamp(newX, minWall, maxWall);

	sf::Vector2f newPos({ newX, unit.stage->get_lane(currentLane).yPos });

	force = std::min(force, 1.2f);
	create_tween(newPos, KNOCKBACK_DURATION * force, RequestType::KNOCKBACK);
	if (tween) tween->easingFuncX = EasingType::OUT_CUBIC;

	unit.anim.start(UnitAnimationState::KNOCKBACK);
}
bool UnitMovement::try_leap(Unit& unit) {
	for (auto const [leftEdge, rightEdge] : unit.stage->get_lane(currentLane).gaps) {
		float leapRange = unit.stats->get_augment(LEAP).value;
		float edge = unit.player_team() ? rightEdge : leftEdge;

		float dist = (edge - pos.x) * static_cast<float>(unit.stats->team);
		if (dist <= 0.f || dist > leapRange || over_this_gap({ leftEdge, rightEdge }, pos.x)) continue;

		float landingSpot = edge + LEDGE_SNAP * static_cast<float>(unit.stats->team);
		sf::Vector2f newPos({ landingSpot, pos.y });

		create_tween(newPos, LEAP_DURATION, RequestType::LEAP);
		if (tween) tween->easingFuncX = EasingType::LINEAR;

		unit.anim.start(UnitAnimationState::JUMPING);
		return true;
	}

	return false;
}

// VERTICAL
void UnitMovement::fall(Unit& unit, float newY) {
	sf::Vector2f newPos({ pos.x, newY });

	create_tween(newPos, FALL_DURATION, RequestType::FALL);
	if (tween) tween->easingFuncY = EasingType::IN_OUT_SINE;

	unit.anim.start(UnitAnimationState::FALLING);
}
void UnitMovement::squash(Unit& unit, float newY) {
	sf::Vector2f newPos({ pos.x, newY });

	create_tween(newPos, SQUASH_DURATION, RequestType::SQUASH);
	if (tween) tween->easingFuncY = EasingType::OUT_BOUNCE;

	unit.anim.start(UnitAnimationState::KNOCKBACK);
}
void UnitMovement::launch(Unit& unit, float newY) {
	sf::Vector2f newPos({ pos.x, newY - LAUNCH_FORCE });

	create_tween(newPos, LAUNCHING_DURATION, RequestType::LAUNCH);
	if (tween) tween->easingFuncY = EasingType::OUT_QUART;

	unit.anim.start(UnitAnimationState::KNOCKBACK);
}
void UnitMovement::jump(Unit& unit, float newX) {
	sf::Vector2f newPos({ newX, unit.stage->get_lane(currentLane).yPos });

	create_tween(newPos, JUMP_DURATION, RequestType::JUMP);
	if (tween) {
		tween->easingFuncX = EasingType::LINEAR;
		tween->easingFuncY = EasingType::OUT_BACK;
	}

	unit.anim.start(UnitAnimationState::JUMPING);
}

// EITHER
void UnitMovement::warp(Unit& unit, const UnitStats* enemyStats) {
	Augment aug = enemyStats->get_augment(WARP);

	currentLane = std::clamp(currentLane + aug.intValue, 0, unit.stage->laneCount - 1);

	float newX = pos.x - (aug.value * (float)unit.stats->team);
	auto [minWall, maxWall] = unit.stage->get_walls(currentLane);
	pos.x = std::clamp(newX, minWall, maxWall);

	unit.combat.cooldown = aug.value2;
}
#pragma endregion

#pragma region Pushing Move Requests
void UnitMovement::push_teleport_request(Unit& unit, const Teleporter& tp) {
	if (!unit.stage->can_push_move_request(unit.id)) return;

	unit.anim.start(UnitAnimationState::MOVE);

	if (tp.connectedLane != currentLane)
		unit.stage->push_move_request(unit, tp.connectedLane, tp.xDestination, RequestType::TELEPORT);
	else pos = { tp.xDestination, unit.stage->lanes[tp.connectedLane].yPos };
}
void UnitMovement::push_fall_request(Unit& unit) {
	const auto [yPos, lane] = unit.stage->find_lane_to_fall_on(unit);

	// Lane will be -1 if no lane to fall on is found 
	// (aka, they fell through ALL lanes, straight into the void)

	if (lane <= -1) {
		unit.status.hp = 0;
		fall(unit, yPos);
	}
	else
		unit.stage->push_move_request(unit, lane, yPos, RequestType::FALL);
}
void UnitMovement::push_squash_request(Unit& unit) {
	if (currentLane == 0) {
		knockback(unit, false);
		return;
	}

	int lane = unit.stage->find_lane_to_knock_to(unit, -1);
	float laneYPos = unit.stage->get_lane(lane).yPos;

	if (lane == currentLane) knockback(unit, false);
	else
		unit.stage->push_move_request(unit, lane, laneYPos, RequestType::SQUASH);
}
void UnitMovement::push_launch_request(Unit& unit) {
	if (currentLane == unit.stage->laneCount - 1 || unit.stats->has_augment(HEAVYWEIGHT)) {
		knockback(unit, false);
		return;
	}

	int lane = unit.stage->find_lane_to_knock_to(unit, 1);
	float laneYPos = unit.stage->get_lane(lane).yPos;

	if (lane == currentLane) knockback(unit, false);
	else
		unit.stage->push_move_request(unit, lane, laneYPos, RequestType::LAUNCH);
}
bool UnitMovement::try_push_jump_request(Unit& unit) const {
	int targetLane = currentLane + 1;
	if (unit.stage->out_of_lane(targetLane, pos.x)) return false;

	float jumpRange = unit.stats->get_augment(JUMP).value;
	Lane& tarLane = unit.stage->get_lane(targetLane);

	for (auto const [leftEdge, rightEdge] : tarLane.gaps) {
		if (!over_this_gap({ leftEdge, rightEdge }, pos.x)) continue; 
		// only jump to gaps you are directly under

		float edge = unit.player_team() ? rightEdge : leftEdge;
		float dist = (edge - pos.x) * static_cast<float>(unit.stats->team);

		if (dist <= 0 || dist > jumpRange) return false;

		float landingSpot = edge + LEDGE_SNAP * static_cast<float>(unit.stats->team);
		unit.stage->push_move_request(unit, targetLane, landingSpot, RequestType::JUMP);
		return true;
	}
	return false;
}
void UnitMovement::finish_launch_tween(Unit& unit) {
	float laneYPos = unit.stage->get_lane(currentLane).yPos;
	sf::Vector2f newPos = { pos.x, laneYPos };

	create_tween(newPos, DROPPING_DURATION, RequestType::NONE);
	if (tween) tween->easingFuncY = EasingType::OUT_BOUNCE;
}
#pragma endregion