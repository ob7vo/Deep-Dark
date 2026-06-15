#pragma once
#include "UnitStats.h"
#include "Animation.h"

// Shared data for Units of the same type during gameplay.
struct SharedUnitData {
	/// <summary> 
	/// The number Unit Instances are using this data.
	/// Units spawned normally (From the player's loadout or from an Enemy Spawner) are not kept track of.
	/// </summary>
	int count = 0;
	const UnitStats stats;
	UnitAnimMap ani;

	SharedUnitData(const nlohmann::json& file, float mag) :
		stats(UnitStats::create_enemy(file, mag)) {
	}
};
