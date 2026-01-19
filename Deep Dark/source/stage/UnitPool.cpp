#include "pch.h"
#include "UnitPool.h"

UnitPool::UnitPool(Stage* stage) {
	for (int i = 0; i < MAX_UNITS; i++)
		pool[i].stage = pool[i].combat.stage = stage;

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
std::optional<size_t> UnitPool::find_unit_index(int spawnID) {
	for (size_t i = 0; i < MAX_UNITS; i++) 
		if (!available[i] && pool[i].spawnID == spawnID) 
			return i;

	return std::nullopt;
}
Unit* UnitPool::find_unit(int spawnID) {
	if (auto index = find_unit_index(spawnID))
		return &pool[*index];

	return nullptr;
}
