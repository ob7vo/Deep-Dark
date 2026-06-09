#pragma once
#include <optional>
class Unit;
struct Stage;

enum class UnitLaneTransferRequestType {
	NOT_DONE = -2,
	NONE = -1,
	KNOCKBACK = 0,
	FALL,
	FAST_FALL,
	SQUASH,
	LAUNCH,
	DROP_FROM_LAUNCH,
	JUMP,
	LEAP,
	PHASE,
	TELEPORT,
	WARP
};

struct Stage;

/// <summary>
/// Needed to move Units between vectors (Lanes), and can ONLY be called
/// for that purpose, not just any plain old movement.
/// </summary>
struct UnitLaneTransferRequest {
	int unitId;
	int unitsCurrentLane;
	int newLane;
	int team;
	float axisPos;
	UnitLaneTransferRequestType type;

	UnitLaneTransferRequest(const Unit& unit, int newLane, float axisPos, UnitLaneTransferRequestType type);

	std::optional<size_t> find_unit_to_move(const Stage& stage) const;
	// Takes in the inex of the Unit inside of Lane's vector, not the pool
	void process(Stage* stage, size_t fromIndex) const; 
	void move_unit_by_request(Stage* stage, size_t unitIndex) const;

	inline bool fall_request() const { return type == UnitLaneTransferRequestType::FALL; }
	inline bool fast_fall_request() const { return type == UnitLaneTransferRequestType::FAST_FALL; }
	inline bool teleport_request() const { return type == UnitLaneTransferRequestType::TELEPORT; }
	inline bool squash_request() const { return type == UnitLaneTransferRequestType::SQUASH; }
	inline bool jump_request() const { return type == UnitLaneTransferRequestType::JUMP; }
	inline bool	launch_request() const { return type == UnitLaneTransferRequestType::LAUNCH; }
	inline bool warp_request() const { return type == UnitLaneTransferRequestType::WARP; }
};