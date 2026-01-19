#pragma once
#include <bitset>
#include <array>
#include <Unit.h>

static constexpr int MAX_UNITS = 80;

struct Stage;

struct UnitPool {
	std::array<Unit, MAX_UNITS> pool;
	std::bitset<MAX_UNITS> available = {};

	std::optional<size_t> spawn_unit();
	std::optional<size_t> find_unit_index(int spawnId);
	Unit* find_unit(int spawnId);

	UnitPool(Stage* stage);

	inline Unit& getUnit(size_t i) { return pool[i]; }
	inline const Unit& getUnit(size_t i) const { return pool[i]; }
	inline void freeUnit(size_t i) { available[i] = true; }

///	static std::array<Unit, 100> create_unit_pool();
};