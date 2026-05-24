#include "pch.h"
#include "StageWallet.h"
#include "Unit.h"
// Wallet
bool StageWallet::try_spend_parts(int partsToSpend, StageRecord& rec) {
	if (parts < partsToSpend) return false;

	parts = std::max(parts - partsToSpend, 0);
	rec.add_parts_spent(partsToSpend);
	return true;
}
void StageWallet::gain_parts(int partsToGain, StageRecord& rec) {
	parts = std::min(parts + partsToGain, partsCap);
	rec.add_parts_earned(partsToGain);
}

void StageWallet::collect_parts_from_unit(const Unit& unit, StageRecord& stageRecorder) {
	int partsToGain = unit.stats->parts;
	if (has(unit.status.statusFlags & AugmentType::PLUNDER)) partsToGain *= 2;

	gain_parts(partsToGain, stageRecorder);
}
void StageWallet::upgrade_bag() {
	curLevel++;

	partsPerSecond = (int)std::round((float)partsPerSecond * 1.1f);
	partsCap += (int)std::round((float)basePartsCap * 0.5f);
	upgradeCost += (int)std::round((float)baseUpgradeCost * 0.25f);
}