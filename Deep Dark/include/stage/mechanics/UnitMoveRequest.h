#pragma once
class Unit;
struct Stage;

enum class RequestType {
	NOT_DONE = -2,
	NONE = -1,
	KNOCKBACK = 0,
	FALL,
	SQUASH,
	LAUNCH,
	DROP_FROM_LAUNCH,
	JUMP,
	LEAP,
	PHASE,
	TELEPORT,
};
/// <summary>
/// Needed to move Units between vectors (Lanes), and can ONLY be called
/// for that purpose, not just any plain old movement.
/// </summary>
struct MoveRequest {
	int unitId;
	int currentLane;
	int newLane;
	int team;
	float axisPos;
	RequestType type;

	MoveRequest(const Unit& unit, int newLane, float axisPos, RequestType type);

	void move_unit_by_request(Unit& unit, Stage& stage) const;
	inline bool fall_request() const { return type == RequestType::FALL; }
	inline bool teleport_request() const { return type == RequestType::TELEPORT; }
	inline bool squash_request() const { return type == RequestType::SQUASH; }
	inline bool jump_request() const { return type == RequestType::JUMP; }
	inline bool	launch_request() const { return type == RequestType::LAUNCH; }
};