#include "pch.h"
#include "UnitLaneTransferRequest.h"
#include "Stage.h"

std::optional<size_t> UnitLaneTransferRequest::find_unit_to_move(const Stage& stage) const {
	const auto& from = stage.lanes[unitsCurrentLane].getAllyUnits(team);

	for (size_t i = 0; i < from.size(); i++) {
		if (stage.getUnit(from[i]).spawnID == unitId) {
			return i; // return index in from
		}
	}

	return std::nullopt;
}
void UnitLaneTransferRequest::process(Stage* stage, size_t fromIndex) const {
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
void UnitLaneTransferRequest::move_unit_by_request(Stage* stage, size_t unitIndex) const {
	auto& unit = stage->getUnit(unitIndex);

	// Cencel any ongoing tweens as a safety-net. Set new Lane
	unit.movement.cancel_tween();
	unit.movement.laneIdx = newLane;

	switch (type) {
	case UnitLaneTransferRequestType::FALL:
	case UnitLaneTransferRequestType::FAST_FALL:
		unit.movement.fall(axisPos, type);
		break;
	case UnitLaneTransferRequestType::SQUASH:
		unit.movement.squash(axisPos);
		break;
	case UnitLaneTransferRequestType::JUMP:
		unit.movement.jump(axisPos);
		break;
	case UnitLaneTransferRequestType::TELEPORT:
		unit.movement.pos = { axisPos, stage->lanes[newLane].yPos };
		break;
	case UnitLaneTransferRequestType::WARP:
		unit.movement.warp(axisPos);
		break;
	default:
		unit.movement.launch(axisPos);
		break;
	}
}
