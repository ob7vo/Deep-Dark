#pragma once
#include "UnitTween.h"

struct Stage;
struct UnitStats;
class Unit;
struct Teleporter;
struct Augment;

struct UnitMovement {
private:
	UnitTween tween;
	Unit& owner;
public:
	Stage* stage = nullptr;;
	int laneIdx = 0;
	sf::Vector2f pos;

	explicit UnitMovement(Unit& ownerUnit) : owner(ownerUnit) {}
	~UnitMovement() = default;

	void setup(sf::Vector2f pos, int lane);

	void knockback(float force = 1.f);
	void shove(bool fromLink = false);
	void squash(float newY);
	void launch(float newY);
	void fall(float newY, UnitLaneTransferRequestType fallType = UnitLaneTransferRequestType::FALL);
	void move(float deltaTime);
	void jump(float newX);
	bool try_leap();
	void warp(float newY);

	// creating tweens
	void queue_fall_request();
	void queue_teleport_request(const Teleporter& tp);
	void queue_squash_request();
	void queue_launch_request();
	void queue_warp_request(const Augment& warpAug, bool fromLinking = false);
	bool try_queue_jump_request()const;

	void finish_launch_tween();
	void create_tween(sf::Vector2f endPos, float time,
		UnitLaneTransferRequestType tweenType, bool overwrite = true);
	UnitLaneTransferRequestType update_tween(float deltaTime);

	inline void cancel_tween() { tween.active = false; };
	inline bool tweening() const { return tween.active; }
};