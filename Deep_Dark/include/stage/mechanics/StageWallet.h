#pragma once
#include "StageRecord.h"
#include <math.h>

const int MAX_BAG_LEVEL = 5;
const int BAG_UPGRADE_PORTION = 4;
const int BASE_BAG_COST = 4;

class Unit;

struct StageWallet {
	int parts = 0; // currency
	int partsPerSecond = 5;

	int curLevel = 1;
	int upgradeCost = 20;
	int baseUpgradeCost = 20;

	int partsCap = 500;
	int basePartsCap = 500;

	StageWallet() = default;

	bool try_spend_parts(int partsToSpend, StageRecord& rec);
	void collect_parts_from_unit(const Unit& unit, StageRecord& rec);
	void gain_parts(int partsToGain, StageRecord& rec);
	void upgrade_bag();

	inline bool can_upgrade_bag(StageRecord& rec) {
		return curLevel < MAX_BAG_LEVEL && try_spend_parts(upgradeCost, rec);
	}
};