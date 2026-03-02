#include "pch.h"
#include "UnitPool.h"

UnitPool::UnitPool(Stage* stage) {
	for (int i = 0; i < MAX_UNITS; i++)
		pool[i].stage = pool[i].combat.stage = pool[i].movement.stage = stage;

	available.set();
}

std::optional<size_t> UnitPool::spawn_unit() {
	for (size_t i = 0; i < MAX_UNITS; i++) {
		if (available[i]) {
			available[i] = false;
			return i;
		}
	}

	return std::nullopt;
}
std::optional<size_t> UnitPool::findIndexBySpawnID(int spawnID) {
	for (size_t i = 0; i < MAX_UNITS; i++) 
		if (!available[i] && pool[i].spawnID == spawnID) 
			return i;

	return std::nullopt;
}
Unit* UnitPool::findUnitBySpawnID(int spawnID) {
	if (auto index = findIndexBySpawnID(spawnID))
		return &pool[*index];

	return nullptr;
}
Unit* UnitPool::findUnitByIndex(size_t index, int spawnID) {
	if (!available[index] && pool[index].spawnID == spawnID)
		return &pool[index];

	return nullptr;
}
