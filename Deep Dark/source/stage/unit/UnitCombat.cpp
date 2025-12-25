#include "pch.h"
#include "UnitCombat.h"
#include "Unit.h"
#include "Stage.h"
#include "Surge.h"

#pragma region Attacking Functions
void UnitCombat::attack(Unit& attacker) {
	cooldown = attacker.stats->attackTime;

	attacker.stage->create_hitbox_visualizers(
		attacker.get_pos(), attacker.get_attack_range(), attacker.stats->team);

	bool hitEnemy = process_attack_on_lanes(attacker);
	handle_post_attack_effects(attacker, hitEnemy);

	hitIndex = (hitIndex + 1) % attacker.stats->totalHits;
}
bool UnitCombat::process_attack_on_lanes(Unit& attacker) const{
	auto [minLane, maxLane] = attacker.get_lane_reach();
	bool hitEnemy = false;

	for (int laneInd = minLane; laneInd <= maxLane; laneInd++) 
		hitEnemy |= attack_lane(attacker, laneInd);

	return hitEnemy;
}
bool UnitCombat::attack_lane(Unit& attacker, int laneIndex) const{
	std::vector<Unit>& enemies = attacker.stage->lanes[laneIndex].getOpponentUnits(attacker.stats->team);

	if (attacker.stats->singleTarget)
		return attack_single_target(attacker, enemies);
	else
		return attack_all_targets(attacker, enemies);
}
bool UnitCombat::attack_single_target(Unit& attacker, std::vector<Unit>& enemies) const{
	Stage& stage = *attacker.stage;
	Unit* singleTargetedUnit = nullptr;

	auto [minAttackRange, maxAttackRange] = attacker.get_attack_range();
	float minDist = abs(attacker.get_pos().x - stage.get_enemy_base(attacker.stats->team).xPos());

	for (auto it = enemies.begin(); it != enemies.end(); ++it) {
		if (attacker.found_valid_target(*it, minAttackRange, maxAttackRange)) {
			float dist = abs(attacker.get_pos().x - it->get_pos().x);
			if (dist < minDist) {
				minDist = dist;
				singleTargetedUnit = std::to_address(it);
			}
		}
	}

	if (singleTargetedUnit && attacker.stats->singleTarget) {
		bool killed = singleTargetedUnit->status.take_damage(*singleTargetedUnit, attacker);
		if (killed) {
			on_kill(attacker, *singleTargetedUnit);
			return true;
		}
	}

	return false;
}
bool UnitCombat::attack_all_targets(Unit& attacker, std::vector<Unit>& enemies) const{
	bool hitEnemy = false;
	auto [minAttackRange, maxAttackRange] = attacker.get_attack_range();

	for (auto it = enemies.begin(); it != enemies.end(); ++it) {
		if (attacker.found_valid_target(*it, minAttackRange, maxAttackRange)) {
			hitEnemy = true;
			if (it->status.take_damage(*it, attacker)) on_kill(attacker, *it);
		}
	}

	return hitEnemy;
}
#pragma endregion

#pragma region Post Attack Functions
void UnitCombat::handle_post_attack_effects(Unit& attacker, bool hitEnemy) const{
	try_attack_enemy_base(attacker, hitEnemy);
	try_create_surge(attacker, hitEnemy);
	try_create_projectile(attacker);
}
void UnitCombat::try_create_surge(Unit& attacker, bool hitEnemy) const{
	if (!hitEnemy || !attacker.stats->has_surge()) return;

	for (auto& augment : attacker.stats->augments) {
		if (attacker.can_make_surge(augment)) {
			bool surgeExist = attacker.stage->create_surge(attacker, augment); // returns pointer

			if (!surgeExist || augment.augType != AugmentType::ORBITAL_STRIKE) continue;

			// Create additional orbital strikes with spacing
			int additionalStrikes = augment.surgeLevel - 1;
			float strikeSpacing = augment.value2;
			float currentOffset = strikeSpacing;

			for (int i = 0; i < additionalStrikes; i++) {
				Surge* strike = attacker.stage->create_surge(attacker, augment);

				strike->pos.x += currentOffset;
				currentOffset += strikeSpacing;
			}
		}
	}
}
void UnitCombat::try_attack_enemy_base(Unit& attacker, bool& hitEnemy) const{
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
void UnitCombat::try_create_projectile(Unit& attacker) const{
	if (!attacker.stats->has_augment(AugmentType::PROJECTILE)) return;

	for (auto& aug : attacker.stats->augments)
		if (has(aug.augType & AugmentType::PROJECTILE) && aug.can_hit(hitIndex))
			attacker.stage->create_projectile(attacker, aug);
}

bool UnitCombat::try_terminate_unit(const Unit& attacker, const Unit& hitUnit, int dmg) const {
	if (!attacker.stats->has_augment(AugmentType::TERMINATE)) return false;

	float threshold = attacker.stats->get_augment(AugmentType::TERMINATE)->value;
	float curHpPercent = (float)(hitUnit.status.hp - dmg) / (float)hitUnit.stats->maxHp;

	return curHpPercent <= threshold;
}

#pragma endregion

void UnitCombat::self_destruct(Unit& explodingUnit, const Augment& selfDestruct) const {
	int minLane = std::max(0, explodingUnit.movement.currentLane - selfDestruct.intValue);
	int maxLane = std::min(explodingUnit.stage->laneCount - 1,
		explodingUnit.movement.currentLane + selfDestruct.intValue);

	for (int i = minLane; i <= maxLane; i++) {
		auto& lane = explodingUnit.stage->lanes[i];
		auto& enemies = lane.getOpponentUnits(explodingUnit.stats->team);

		for (auto it = enemies.begin(); it != enemies.end(); ++it) {
			if (explodingUnit.found_valid_target(*it, selfDestruct.value2, selfDestruct.value2)
				&& it->status.take_damage(*it, (int)selfDestruct.value))  {
				on_kill(explodingUnit, *it);
			}
		}
	}
}
void UnitCombat::on_kill(Unit& attacker, Unit& enemyUnit) const {
	if (attacker.stats->try_proc_augment(AugmentType::PLUNDER))
		enemyUnit.status.statusFlags |= AugmentType::PLUNDER;

	if (attacker.stats->has_augment(AugmentType::CODE_BREAKER))
		enemyUnit.status.statusFlags |= AugmentType::CODE_BREAKER;

	if (attacker.stats->has_augment(AugmentType::SALVAGE))
		attacker.stage->create_summon(attacker);

	enemyUnit.causeOfDeath |= DeathCause::UNIT;
}