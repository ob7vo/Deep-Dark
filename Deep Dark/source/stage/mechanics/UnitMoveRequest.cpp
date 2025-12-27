#include "pch.h"
#include "UnitMoveRequest.h"
#include "Stage.h"

std::optional<size_t> UnitMoveRequest::find_unit_to_move(const Stage& stage) const {
	const auto& from = stage.lanes[unitsCurrentLane].getAllyUnits(team);

	const auto unit = std::find_if(from.begin(), from.end(),
		[id = unitId](const Unit& u) { return u.id == id; });

	// Return null if the Unit wasn't found or the lanes are the same
	if (unit == from.end()) {
		std::cout << "Unit was not found in vector." << std::endl;
		return std::nullopt;
	}
	else if (unitsCurrentLane == newLane) {
		std::cout << "MoveRequest moves Unit to the same lane." << std::endl;
		return std::nullopt;
	}

	return std::distance(from.begin(), unit);
}
void UnitMoveRequest::process(Stage& stage, size_t unitToMoveIndex) const {
	auto& from = stage.lanes[unitsCurrentLane].getAllyUnits(team);
	auto& unitToMove = from[unitToMoveIndex];

	// Cencel any ongoing tweens as a safety-net 
	unitToMove.movement.cancel_tween();

	// Swap the Unit between Lanes
	auto& to = stage.lanes[newLane].getAllyUnits(team);
	Unit& movedUnit = to.emplace_back(std::move(unitToMove));
	from.erase(from.begin() + unitToMoveIndex);

	move_unit_by_request(movedUnit, stage);
};