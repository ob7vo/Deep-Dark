#pragma once
#include "Tween.h"

struct Stage;
struct UnitStats;
class Unit;
struct Teleporter;

struct UnitMovement {
	int currentLane = 0;
	sf::Vector2f pos;
	std::unique_ptr<UnitTween> tween = nullptr;

	UnitMovement(sf::Vector2f pos, int lane);

	void knockback(Unit& unit, float force = 1.f);
	void squash(Unit& unit, float newY);
	void launch(Unit& unit, float newY);
	void fall(Unit& unit, float newY);
	void move(Unit& unit, float deltaTime);
	void jump(Unit& unit, float newX);
	bool try_leap(Unit& unit);
	void warp(Unit& unit, const UnitStats* enemyStats);

	// creating tweens
	void push_fall_request(Unit& unit);
	void push_teleport_request(Unit& unit, const Teleporter& tp);
	void push_squash_request(Unit& unit);
	void push_launch_request(Unit& unit);
	bool try_push_jump_request(Unit& unit)const;

	void finish_launch_tween(Unit& unit);
	void create_tween(sf::Vector2f endPos, float time,
		RequestType tweenType, bool overwrite = true);

	RequestType update_tween(float deltaTime);
	inline void cancel_tween() { tween = nullptr; };
	inline bool tweening() const { return tween != nullptr; }
};