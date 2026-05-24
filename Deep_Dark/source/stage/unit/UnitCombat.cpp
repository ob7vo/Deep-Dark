#include "pch.h"
#include "UnitCombat.h"
#include "Unit.h"
#include "Stage.h"
#include "Surge.h"

#pragma region Attacking Functions
void UnitCombat::attack() {
	cooldown = attacker.stats->attackTime;

	attacker.stage->create_hitbox_visualizers(
		attacker.movement.pos, attacker.get_attack_range(), attacker.stats->team);

	bool didHitEnemy = process_attack_on_lanes();
	handle_post_attack_effects(didHitEnemy);

	hitIndex = (hitIndex + 1) % attacker.stats->totalHits;
}
bool UnitCombat::process_attack_on_lanes() {
	auto [minLane, maxLane] = attacker.get_lane_reach();
	bool hitEnemy = false;

	for (int laneInd = minLane; laneInd <= maxLane; laneInd++) 
		hitEnemy |= attack_lane(laneInd);

	return hitEnemy;
}
bool UnitCombat::attack_lane(int laneIndex) {
	const auto& enemyIndexes = stage->lanes[laneIndex].getOpponentUnits(attacker.stats->team);

	if (attacker.stats->singleTarget)
		return attack_single_target(enemyIndexes);
	else
		return attack_all_targets(enemyIndexes);
}
bool UnitCombat::attack_single_target(const std::vector<size_t>& enemyIndexes) {
	size_t targetIndex = NULL_UNIT_INDEX;

	auto [minAttackRange, maxAttackRange] = attacker.get_attack_range();
	float minDist = abs(attacker.movement.pos.x - stage->get_enemy_base(attacker.stats->team).xPos());

	for (const auto& index : enemyIndexes) {
		const auto& enemyUnit = stage->getUnit(index);

		if (attacker.found_valid_target(enemyUnit, minAttackRange, maxAttackRange)) {
			float dist = abs(attacker.movement.pos.x - enemyUnit.movement.pos.x);

			if (dist < minDist) {
				minDist = dist;
				targetIndex = enemyUnit.spawnID;
			}
		}
	}

	if (targetIndex != NULL_UNIT_INDEX) {
		auto& hitUnit = stage->getUnit(targetIndex);

		if (hitUnit.status.take_damage(attacker)) {
			on_kill(hitUnit);
			return true;
		}
	}

	return false;
}
bool UnitCombat::attack_all_targets(const std::vector<size_t>& enemyIndexes) {
	bool hitEnemy = false;
	auto [minAttackRange, maxAttackRange] = attacker.get_attack_range();

	for (const auto& index : enemyIndexes) {
		auto& enemyUnit = stage->getUnit(index);

		if (attacker.found_valid_target(enemyUnit, minAttackRange, maxAttackRange)) {
			hitEnemy = true;
			if (enemyUnit.status.take_damage(attacker)) 
				on_kill(enemyUnit);
		}
	}

	return hitEnemy;
}
#pragma endregion

#pragma region Post Attack Functions
void UnitCombat::handle_post_attack_effects(bool hitEnemy) const{
	try_attack_enemy_base(hitEnemy);
	try_create_surge(hitEnemy);
	try_create_projectile();
}

void UnitCombat::try_create_surge(bool hitEnemy) const{
	if (!hitEnemy || !attacker.stats->has_surge()) return;

	for (auto& augment : attacker.stats->augments) {
		if (attacker.can_make_surge(augment)) {
			bool surgeExist = stage->create_surge(attacker, augment); // returns pointer

			if (!surgeExist || augment.augType != AugmentType::ORBITAL_STRIKE) continue;

			// Create additional orbital strikes with spacing
			int additionalStrikes = augment.surgeLevel - 1;
			float strikeSpacing = augment.value2;
			float currentOffset = strikeSpacing;

			for (int i = 0; i < additionalStrikes; i++) {
				Surge* strike = stage->create_surge(attacker, augment);

				strike->pos.x += currentOffset;
				currentOffset += strikeSpacing;
			}
		}
	}
}
void UnitCombat::try_attack_enemy_base(bool& hitEnemy) const {
	// if the Unit is single target and has already hit an enemy.
	if (attacker.stats->singleTarget && hitEnemy) return; 

	Base& enemyBase = attacker.stage->get_enemy_base(attacker.stats->team);
	if (enemyBase.destroyed()) return;

	auto [minRange, maxRange] = attacker.get_attack_range();

	// I need to set hitEnemy Reference to true here, as in the case the the ATTACKER
	// Is single target and has not hit an enemy, the base can count for a hit enemy,
	// Thus allowing the ATTACK to run try_create_surge() directly after this
	if (attacker.enemy_in_range(enemyBase.xPos(), minRange, maxRange)) {
		enemyBase.take_damage(attacker.stage, attacker.get_dmg());
		hitEnemy = true;
	}
}
void UnitCombat::try_create_projectile() const {
	if (!attacker.stats->has_augment(AugmentType::PROJECTILE)) return;

	for (auto& aug : attacker.stats->augments)
		if (has(aug.augType & AugmentType::PROJECTILE) && aug.can_hit(hitIndex))
			stage->create_projectile(attacker, aug);
}
bool UnitCombat::try_terminate_unit(const Unit& hitUnit, int dmg) const{
	if (!attacker.stats->has_augment(AugmentType::TERMINATE)) return false;

	float threshold = attacker.stats->get_augment(AugmentType::TERMINATE)->value;
	float curHpPercent = (float)(hitUnit.status.hp - dmg) / (float)hitUnit.stats->maxHp;

	return curHpPercent <= threshold;
}
#pragma endregion

void UnitCombat::self_destruct(const Augment& selfDestruct) {
	int minLane = std::max(0, attacker.movement.laneInd - selfDestruct.intValue);
	int maxLane = std::min(stage->laneCount - 1, attacker.movement.laneInd + selfDestruct.intValue);

	for (int i = minLane; i <= maxLane; i++) {
		auto& lane = stage->lanes[i];
		const auto& enemyIndexes = lane.getOpponentUnits(attacker.stats->team);

		for (const auto& index : enemyIndexes) {
			auto& enemyUnit = stage->getUnit(index);

			if (attacker.found_valid_target(enemyUnit, selfDestruct.value2, selfDestruct.value2)
				&& enemyUnit.status.take_damage((int)selfDestruct.value))
			{
				on_kill(enemyUnit);
			}
		}
	}
}
void UnitCombat::discharge(int oldHp) {
	int totalHPLost = oldHp - attacker.status.hp;
	float explosionRange = attacker.stats->sightRange * UnitConfig::CHARGE_RELEASE_RANGE_BOOST;

	for (const auto& index : attacker.getLaneEnemies()) {
		Unit& enemyUnit = attacker.stage->getUnit(index);

		if (std::abs(attacker.movement.pos.x - enemyUnit.movement.pos.x) <= explosionRange &&
			enemyUnit.status.take_damage(totalHPLost))
		{
			on_kill(enemyUnit);
		}
	}
}

void UnitCombat::on_kill(Unit& enemyUnit) {
	kills++;

	if (attacker.stats->try_proc_augment(AugmentType::PLUNDER))
		enemyUnit.status.statusFlags |= AugmentType::PLUNDER;

	if (attacker.stats->has_augment(AugmentType::CODE_BREAKER))
		enemyUnit.status.statusFlags |= AugmentType::CODE_BREAKER;

	if (attacker.stats->has_augment(AugmentType::SALVAGE))
		attacker.stage->create_summon(attacker);

	if (const auto syphon = attacker.stats->get_augment(AugmentType::SYPHON)) {
		if (kills % syphon->intValue == 0)
			attacker.status.syphon(syphon);
	}

	enemyUnit.causeOfDeath |= DeathCause::UNIT;
}