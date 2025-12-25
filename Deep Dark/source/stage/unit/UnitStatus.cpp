#include "pch.h"
#include "UnitStatus.h"
#include "Unit.h"
#include "Stage.h"
#include "Surge.h"
#include "Utils.h"

const float STATUS_ICON_SPACING = 40.f;

UnitStatus::UnitStatus(const UnitStats* stats) {
	activeStatuses.reserve(MAX_EFFECTS);
	hp = stats->maxHp;
	kbIndex = 1;


	if (auto shield = stats->get_augment(AugmentType::SHIELD))
		shieldHp = static_cast<int>(shield->value);

	//tries to get augment to flip status mask. If it doesnt have the Augment,
	// it will give an empty Augment with NONE, or 0
	statusFlags = AugmentType::NONE;

	statusFlags |= stats->has_augment(AugmentType::SURVIVOR) ? AugmentType::SURVIVOR : AugmentType::NONE;
	statusFlags |= stats->has_augment(AugmentType::PHASE) ? AugmentType::PHASE : AugmentType::NONE;
	statusFlags |= stats->has_augment(AugmentType::TRANSFORM) ? AugmentType::TRANSFORM : AugmentType::NONE;
}

#pragma region Calculations
float UnitStatus::calculate_damage_reduction(const std::vector<Augment>& augments) const {
	// called from attacked Unit, parameters are from the attackING unit.
	float boost = 1;

	for (const Augment& augment : augments) {
		if (!augment.is_damage_modifier()) continue;

		if (has(augment.augType & AugmentType::RESIST))
			boost *= augment.percentage;
		else if (has(augment.augType & AugmentType::SUPERIOR))
			boost /= augment.percentage;
	}

	return boost;
}
int UnitStatus::calculate_damage_and_effects(Unit& hitUnit, const Unit& attacker) {
	// called from attacked Unit, parameters are from the attackING unit.
	// Run calculations from ABILITIES and DEBUFFS
	int dmg = attacker.get_dmg();
	dmg = corroded() ? (int)(static_cast<float>(dmg) * 2.f) : dmg;
	dmg = attacker.status.weakened() ? (int)(static_cast<float>(dmg) * .5f) : dmg;

	// If the ATTACKING Unit targets this unit's trait, run its damage-augments
	if (hitUnit.targeted_by_unit(attacker))
		dmg = apply_effects(hitUnit, attacker.stats->augments, attacker.combat.hitIndex, dmg);

	// If thi Unit targets the ATTACKING Unit's trait, run this unit's damage-augments
	if (attacker.targeted_by_unit(hitUnit))
		dmg = (int)((float)dmg * calculate_damage_reduction(hitUnit.stats->augments));

	// return if the unit has a shield and ti did not break
	// Try break the Shield BEFORE void and Terminate, as they do NOT go through shields
	if (has_shield_up() && !damage_shield(dmg, attacker.stats)) return 0;

	// These effects are based around the Unit's current HP, 
	// so they are run after all calculations
	if (hitUnit.targeted_by_unit(attacker)) {
		if (attacker.stats->try_proc_augment(AugmentType::VOID, attacker.combat.hitIndex))
			dmg += (int)((float)hitUnit.stats->maxHp * attacker.stats->get_augment(AugmentType::VOID)->value);
		if (attacker.combat.try_terminate_unit(attacker, hitUnit, dmg))
			dmg += hitUnit.stats->maxHp;
	}

	return dmg;
}
#pragma endregion

#pragma region Status Effects
void UnitStatus::add_status_effect(const Augment& aug) {
	// Remove existing effect of same type (so new slow overrides old)
	if (!aug.is_status_effect()) {
		std::cout << "Augment type was not a status. Cannot add effect" << std::endl;
		return;
	}

	StatusEffect newEffect(aug.augType, aug.value);
	std::cout << "adding effect, bit mask value: " << (uint32_t)aug.augType << std::endl;

	// If the Status effect already exist, remove the old one to later replace it
	activeStatuses.erase(
		std::remove_if(activeStatuses.begin(), activeStatuses.end(),
			[&](const StatusEffect& existing) {
				if (existing.effect == newEffect.effect) {
					remove(statusFlags, existing.effect);
					return true;
				}
				return false;
			}),
		activeStatuses.end()
	);

	// If there are too many Status Effects
	if (activeStatuses.size() >= MAX_EFFECTS) {
		// STRENGTHEN will ALWAYS be applied, so replace the newest effect
		if (has(newEffect.effect & AugmentType::STRENGTHEN)) {
			remove(statusFlags, activeStatuses.back().effect);
			statusFlags |= AugmentType::STRENGTHEN;

			activeStatuses.pop_back();
			activeStatuses.push_back(newEffect);
		}
		else return;
	}

	statusFlags |= newEffect.effect;
	activeStatuses.push_back(newEffect);
}
void UnitStatus::update_status_effects(Unit& unit, float deltaTime) {
	if (activeStatuses.size() <= 0) return;

	sf::Vector2f statusPos = { unit.get_pos().x, unit.get_pos().y - 20.f};

	for (size_t i = activeStatuses.size(); i--;) {
		auto& status = activeStatuses[i];
		status.update(deltaTime);

		if (status.is_expired()) {
			remove(statusFlags, status.effect);

			activeStatuses[i] = std::move(activeStatuses.back());
			activeStatuses.pop_back();
		}
		else {
			unit.stage->effectSpritePositions.emplace_back(status.effect, statusPos);
			statusPos.x -= STATUS_ICON_SPACING * static_cast<float>(unit.stats->team);
		}
	}
}
int UnitStatus::apply_effects(const Unit& hitUnit, const std::vector<Augment>& augments, 
	int attackersHitIndex, int dmg) {
	// called from attacked Unit, parameters are from the attackING unit.
	for (const Augment& augment : augments) {
		if (can_proc_status(hitUnit, augment, attackersHitIndex)
			&& Random::chance(augment.percentage)) {
			add_status_effect(augment);
		}
		else if (augment.is_damage_modifier()) {
			dmg = (int)((float)dmg * augment.percentage);
		}
		else if (has(augment.augType & AugmentType::CRITICAL) &&
			Random::chance(augment.percentage)) {
			dmg *= 2;
		}
	}

	return dmg;
}
bool UnitStatus::can_proc_status(const Unit& unit, const Augment& augment, int hitIndex) const {
	return augment.is_status_effect() && !unit.immune(augment.augType) &&
		!has_shield_up() && augment.can_hit(hitIndex);
}
#pragma endregion

#pragma region Taking Damage

bool UnitStatus::damage_shield(int& dmg, const UnitStats* _stats) {
	// shield pierce can instantly break shields
	if (_stats && _stats->try_proc_augment(AugmentType::SHIELD_PIERCE)) {
		shieldHp = 0;
		return true;
	}
	else if (dmg >= shieldHp) {
		dmg -= shieldHp;
		shieldHp = 0;

		return true;
	}

	shieldHp -= dmg;
	dmg = 0;
	return false;
}
bool UnitStatus::take_damage(Unit& hitUnit, const Unit& attacker) {
	int dmg = calculate_damage_and_effects(hitUnit, attacker);

	int oldHp = hp;
	hp -= dmg;

	hitUnit.try_knockback(oldHp, attacker.combat.hitIndex, attacker.stats);
	return hp <= 0 && !try_proc_survive(hitUnit.stats);
}
bool UnitStatus::take_damage(Unit& hitUnit, const Surge& surge) {
	int dmg = surge.calculate_damage_and_effects(hitUnit);

	int oldHp = hp;
	hp -= dmg;

	hitUnit.try_knockback(oldHp, surge.hitIndex, surge.stats);

	return hp <= 0 && !try_proc_survive(hitUnit.stats);
}
bool UnitStatus::take_damage(Unit& hitUnit, int dmg, bool shove) {
	dmg = corroded() ? dmg * 2 : dmg;

	if (has_shield_up() && !damage_shield(dmg)) return false; // return if shield did not break

	int oldHp = hp;
	hp -= dmg;

	if (!hitUnit.anim.in_knockback() && met_knockback_threshold(oldHp, hitUnit.stats)) {
		shieldHp = (int)hitUnit.stats->get_augment(AugmentType::SHIELD)->value;
		hitUnit.movement.knockback(hitUnit);
	}
	else if (shove) hitUnit.movement.knockback(hitUnit, 0.5f);

	return hp <= 0 && !try_proc_survive(hitUnit.stats);
}
#pragma endregion


bool UnitStatus::met_knockback_threshold(int oldHp, const UnitStats* stats) {
	bool metThreshold = false;

	for (int i = kbIndex; i <= stats->knockbacks; i++) {
		int threshold = stats->maxHp - (stats->maxHp * i / stats->knockbacks);

		if (oldHp > threshold && hp <= threshold) {
			//std:: << "MET KB THRESHOLD #" << kbIndex << " of hp: " << threshold 
				//" ----- maxHp: " << stats->maxHp << " - oldHp: " << oldHp << " - newHp: " << hp << std::endl;
			metThreshold = true;
			kbIndex = i + 1;
		}
		else break;
	}

	return metThreshold;
}
bool UnitStatus::try_proc_survive(const UnitStats* dyingUnitStats) {
	if (dyingUnitStats->try_proc_augment(AugmentType::SURVIVOR)) {
		remove(statusFlags, AugmentType::SURVIVOR);

		hp = 1;
		kbIndex = dyingUnitStats->knockbacks;
		std::cout << "SURVIVED BITCH" << std::endl;

		return true;
	}
	return false;
}