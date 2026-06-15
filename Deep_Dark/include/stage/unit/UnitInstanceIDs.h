#pragma once

struct UnitInstanceIDs {
	size_t poolIndex;
	// Specific instance of the Unit. Given to the unit when created
	int spawnID;

	UnitInstanceIDs(size_t index, int id) : poolIndex(index), spawnID(id) {}
};