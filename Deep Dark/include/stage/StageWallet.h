#pragma once
#include "StageRecord.h"

const int MAX_BAG_LEVEL = 5;
const int BAG_UPGRADE_PORTION = 4;
const int BASE_BAG_COST = 4;

struct StageWallet {
	int parts = 0; // currency
	int partsPerSecond = 5;

	int curLevel = 1;
	int upgradeCost = 20;
	int baseUpgradeCost = 20;

	int partsCap = 500;
	int basePartsCap = 500;

	StageWallet() = default;

	inline bool try_spend_parts(int partsToSpend, StageRecord& rec) {
		if (parts < partsToSpend) return false;

		parts = std::max(parts - partsToSpend, 0);
		rec.add_parts_spent(partsToSpend);
		return true;
	}
	inline void gain_parts(int partsToGain, StageRecord& rec) {
		parts = std::min(parts + partsToGain, partsCap);
		rec.add_parts_earned(partsToGain);
	}
	inline bool try_buy_upgrade_bag(StageRecord& rec) {
		return curLevel < MAX_BAG_LEVEL && try_spend_parts(upgradeCost, rec);
	}
};