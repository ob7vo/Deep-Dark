#include "pch.h"
#include "UnitMoveRequest.h"
#include "Stage.h"

std::optional<size_t> UnitMoveRequest::find_unit_to_move(const Stage& stage) const {
	const auto& from = stage.lanes[unitsCurrentLane].getAllyUnits(team);

	for (size_t i = 0; i < from.size(); i++) {
		if (stage.getUnit(from[i]).spawnID == unitId) {
			return i; // return index in from
		}
	}

	return std::nullopt;
}
void UnitMoveRequest::process(Stage* stage, size_t fromIndex) const {
	auto& from = stage->lanes[unitsCurrentLane].getAllyUnits(team);
	auto& to = stage->lanes[newLane].getAllyUnits(team);

	// Index (size_t) found in "from" that is sed to get a Unit from the Object Pool
	size_t actualUnitIndex = from[fromIndex];

	// Swap the Unit between Lanes
	to.emplace_back(actualUnitIndex);
	from[fromIndex] = from.back(); // Swap and Pop
	from.pop_back();

	move_unit_by_request(stage, actualUnitIndex);
};
void UnitMoveRequest::move_unit_by_request(Stage* stage, size_t unitIndex) const {
	auto& unit = stage->getUnit(unitIndex);

	// Cencel any ongoing tweens as a safety-net. Set new Lane
	unit.movement.cancel_tween();
	unit.movement.laneInd = newLane;

	switch (type) {
	case UnitMoveRequestType::FALL:
		unit.movement.fall(unit, axisPos);
		break;
	case UnitMoveRequestType::SQUASH:
		unit.movement.squash(unit, axisPos);
		break;
	case UnitMoveRequestType::JUMP:
		unit.movement.jump(stage, unit, axisPos);
		break;
	case UnitMoveRequestType::TELEPORT:
		unit.movement.pos = { axisPos, stage->lanes[newLane].yPos };
		break;
	default:
		unit.movement.launch(unit, axisPos);
		break;
	}
}
