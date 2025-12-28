#pragma once
#include "UnitTween.h"

struct Stage;
struct UnitStats;
class Unit;
struct Teleporter;

struct UnitMovement {
private:
	UnitTween tween;
public:
	int laneInd = 0;
	sf::Vector2f pos;

	UnitMovement() = default;
	~UnitMovement() = default;

	void setup(sf::Vector2f pos, int lane);

	void knockback(const Stage* stage, Unit& unit, float force = 1.f);
	void squash(Unit& unit, float newY);
	void launch(Unit& unit, float newY);
	void fall(Unit& unit, float newY);
	void move(const Unit& unit, float deltaTime);
	void jump(const Stage* stage, Unit& unit, float newX);
	bool try_leap(const Stage* stage, Unit& unit);
	void warp(const Stage* stage, Unit& unit, const UnitStats* enemyStats);

	// creating tweens
	void push_fall_request(Stage* stage, Unit& unit);
	void push_teleport_request(Stage* stage, Unit& unit, const Teleporter& tp);
	void push_squash_request(Stage* stage, Unit& unit);
	void push_launch_request(Stage* stage, Unit& unit);
	bool try_push_jump_request(Stage* stage, Unit& unit)const;

	void finish_launch_tween(const Stage* stage);
	void create_tween(sf::Vector2f endPos, float time,
		UnitMoveRequestType tweenType, bool overwrite = true);
	UnitMoveRequestType update_tween(float deltaTime);

	inline void cancel_tween() { tween.active = false; };
	inline bool tweening() const { return tween.active; }
};