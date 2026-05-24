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
	std::optional<size_t> findIndexBySpawnID(int spawnId);
	/// <summary>
	///  Gets the specific (ALIVE) instance of a unit via their spawn ID
	/// </summary>
	Unit* findUnitBySpawnID(int spawnId);
	/// <summary>
	/// Gets the specific (ALIVE) instance of a unit via their Pool Index
	/// for quicker access than finding a Unit by just their spawn ID
	/// </summary>
	Unit* findUnitByIndex(size_t index, int spawnId);

	UnitPool(Stage* stage);

	inline Unit& getUnit(size_t i) { return pool[i]; }
	inline const Unit& getUnit(size_t i) const { return pool[i]; }
	inline void freeUnit(size_t i) { available[i] = true; }

///	static std::array<Unit, 100> create_unit_pool();
};