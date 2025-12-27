#pragma once
class Unit;
struct Stage;


enum class UnitMoveRequestType {
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

struct Stage;

/// <summary>
/// Needed to move Units between vectors (Lanes), and can ONLY be called
/// for that purpose, not just any plain old movement.
/// </summary>
struct UnitMoveRequest {
	int unitId;
	int unitsCurrentLane;
	int newLane;
	int team;
	float axisPos;
	UnitMoveRequestType type;

	UnitMoveRequest(const Unit& unit, int newLane, float axisPos, UnitMoveRequestType type);

	std::optional<size_t> find_unit_to_move(const Stage& stage) const;
	void process(Stage& stage, size_t unitToMoveIndex) const; 
	void move_unit_by_request(Unit& unit, Stage& stage) const;

	inline bool fall_request() const { return type == UnitMoveRequestType::FALL; }
	inline bool teleport_request() const { return type == UnitMoveRequestType::TELEPORT; }
	inline bool squash_request() const { return type == UnitMoveRequestType::SQUASH; }
	inline bool jump_request() const { return type == UnitMoveRequestType::JUMP; }
	inline bool	launch_request() const { return type == UnitMoveRequestType::LAUNCH; }
};