#include "pch.h"
#include "UnitStatus.h"
#include "Unit.h"
#include "Stage.h"
#include "Surge.h"
#include "Utils.h"

const float STATUS_ICON_SPACING = 40.f;

void UnitStatus::setup(const UnitStats* stats) {
	activeStatuses.clear();
	activeStatuses.reserve(MAX_EFFECTS);

	hp = stats->maxHp;
	kbIndex = 1;

	//tries to get augment to flip status mask. If it doesnt have the Augment,
	// it will give an empty Augment with NONE, or 0
	statusFlags = AugmentType::NONE;

	statusFlags |= stats->has_augment(AugmentType::SURVIVOR) ? AugmentType::SURVIVOR : AugmentType::NONE;
	statusFlags |= stats->has_augment(AugmentType::PHASE) ? AugmentType::PHASE : AugmentType::NONE;
	statusFlags |= stats->has_augment(AugmentType::TRANSFORM) ? AugmentType::TRANSFORM : AugmentType::NONE;
}

#pragma region Calculations
float UnitStatus::calculate_damage_boost(const std::vector<Augment>& augments) const {
	float boost = 1;

	for (const Augment& augment : augments) {
		if (augment.is_damage_modifier())
			boost *= augment.data.damage.dmgMultiplier;
		else if (has(augment.augType & AugmentType::CRITICAL)
			&& Random::chance(augment.activationChance))
			boost *= 2;
	}
	
	return boost;
}
int UnitStatus::calculate_damage_taken_and_apply_augments(const Unit& attacker) {
	// called from attacked Unit, parameters are from the attackING unit.
	// Run calculations from ABILITIES and DEBUFFS
	float dmg = attacker.get_dmg();

	// Status effect multipliers
	dmg *= get_corrosion_multiplier();
	dmg *= get_reinforcement_multiplier();
	dmg *= attacker.status.get_weaken_multiplier();

	// If thi Unit targets the ATTACKING Unit's trait, try to apply RESIST fi it has it
	if (attacker.targeted_by_unit(owner))
		for (const Augment& augment : owner.stats->augments) {
			if (has(augment.augType & AugmentType::RESIST))
				dmg *= augment.data.damage.dmgMultiplier;
		}

	if (owner.targeted_by_unit(attacker)) {
		apply_on_hit_status_effects(attacker.stats->augments, attacker.combat.hitIndex);
		dmg *= calculate_damage_boost(attacker.stats->augments);

		// These effects are based around the Unit's current HP, 
		// so they are run after all calculations

		// Run VOID check
		if (attacker.stats->try_proc_augment(AugmentType::VOID, attacker.combat.hitIndex))
			dmg += (float)owner.stats->maxHp * attacker.stats->get_augment(AugmentType::VOID)->data.general.magnitude;
		// Run TERMINATE check
		if (attacker.combat.try_terminate_unit(owner, dmg))
			dmg += (float)owner.stats->maxHp;
	}

	return static_cast<int>(dmg);
}
#pragma endregion

#pragma region Status Effects
std::optional<StatusEffect> UnitStatus::get_status_effect(AugmentType effType) const {
	for (const auto& statusEff : activeStatuses)
		if (statusEff.effect == effType)
			return statusEff;

	return std::nullopt;
}
// Linking is when a unit applies there status affect to another unit
// Linked boolean is needed to stop infinite linking
void UnitStatus::process_new_status_effect(const Augment& aug, bool fromLink) {
	// Remove existing effect of same type (so new slow overrides old)
	if (!aug.is_status_effect()) {
		std::cout << "Augment type was not a status. Cannot add effect" << std::endl;
		return;
	}
	
	float duration = aug.is_negative_status() ? aug.data.status.duration : 9999.f;
	StatusEffect newEffect(aug.augType, aug.get_magnitude(), duration);
	
	add_status_effect(newEffect);

	if (!fromLink && has(owner.stats->augmentsMask, AugmentType::LINK))
		link_augment(aug);
}
void UnitStatus::add_status_effect(const StatusEffect& statusEffect) {
	// If the Status effect already exist, remove the old one to later replace it
	activeStatuses.erase(
		std::remove_if(activeStatuses.begin(), activeStatuses.end(),
			[&](const StatusEffect& existing) {
				if (existing.effect == statusEffect.effect) {
					remove(statusFlags, existing.effect);
					return true;
				}
				return false;
			}),
		activeStatuses.end()
	);

	// If there are too many Status Effects, Remove the oldest one
	// This does allow postive statuses to be replaced. This is intentional (for now)
	if (activeStatuses.size() >= MAX_EFFECTS) {
		remove(statusFlags, activeStatuses.back().effect);
		activeStatuses.pop_back();
	}

	statusFlags |= statusEffect.effect;
	activeStatuses.push_back(statusEffect);
}
void UnitStatus::trigger_health_threshold_augments() {
	float hpPercentage = static_cast<float>(hp) / static_cast<float>(owner.stats->maxHp);

	for (const auto& augment : owner.stats->augments) {
		if (augment.activates_via_health_threshold() && hpPercentage <= augment.data.onHPThreshold.hpPercentage) {
			process_new_status_effect(augment);
		}
	}
}
void UnitStatus::link_augment(const Augment& augmentToLink) {
	Stage* stage = owner.stage;

	// Already checked for LINK to get here.
	const Augment& link = *owner.stats->get_augment(AugmentType::LINK);
	float linkRange = link.data.link.range;
	int maxLane = std::min(stage->laneCount - 1, owner.get_lane() + link.data.link.reachesAdjacentLanes);
	int minLane = std::max(0, owner.get_lane() - link.data.link.reachesAdjacentLanes);

	// Get link Target Team returns 1 if it targets allies, adn -1 for enemies. I wanna rename this too
	bool targetTeam = owner.stats->team * Augment::get_link_target_team(augmentToLink.augType);

	bool isStatusEffect = augmentToLink.is_status_effect();
	bool isSyphon = has(augmentToLink.augType, AugmentType::SYPHON);
	bool isShove = has(augmentToLink.augType, AugmentType::SHOVE);
	bool isWarp = has(augmentToLink.augType, AugmentType::WARP);

	for (int i = minLane; i <= maxLane; i++) {
		const auto& unitIndexes = stage->lanes[i].getAllyUnits(targetTeam);

		for (const auto& index : unitIndexes) {
			Unit& unit = stage->getUnit(index);

			if (unit.spawnID == owner.spawnID ||
				!owner.enemy_in_range(unit.movement.pos.x, -linkRange, linkRange))
				continue; // Don't link to self or units out of range

			if (isStatusEffect)
				unit.status.process_new_status_effect(augmentToLink, true);
			else if (isSyphon)
				unit.status.syphon(&augmentToLink, true);
			// Invincibility check since these augments would interupt some animation states otherwise (Like FALLING, or KNOCKBACK)
			// I could make sure they don't lead to buggy behavior, but this is easier
			else if (!unit.anim.invincible()) {
				if (isShove)
					unit.movement.shove(true);
				else if (isWarp)
					unit.movement.queue_warp_request(augmentToLink, true);
			}
		}
	}
}
void UnitStatus::update_status_effects(float deltaTime) {
	if (activeStatuses.size() <= 0) return;

	sf::Vector2f statusPos = { owner.movement.pos.x, owner.movement.pos.y - 20.f};

	for (size_t i = activeStatuses.size(); i--;) {
		auto& status = activeStatuses[i];
		status.update(deltaTime);

		if (status.is_expired()) {
			remove(statusFlags, status.effect);

			activeStatuses[i] = std::move(activeStatuses.back());
			activeStatuses.pop_back();
		}
		else {
			owner.stage->effectSpritePositions.emplace_back(status.effect, statusPos);
			statusPos.x -= STATUS_ICON_SPACING * static_cast<float>(owner.stats->team);
		}
	}
}
void UnitStatus::apply_on_hit_status_effects(const std::vector<Augment>& augments,int attackersHitIndex) {
	// called from attacked Unit, parameters are from the attackING unit.
	for (const Augment& augment : augments) {
		if (can_proc_status(augment, attackersHitIndex) && Random::chance(augment.activationChance)) {
			process_new_status_effect(augment);
		}
	}
}
bool UnitStatus::can_proc_status(const Augment& augment, int hitIndex) const {
	return augment.is_negative_status() && !owner.immune(augment.augType) &&
		augment.can_hit(hitIndex);
}
#pragma endregion


#pragma region Taking Damage
bool UnitStatus::take_damage(const Unit& attackerUnit) {
	int dmg = calculate_damage_taken_and_apply_augments(attackerUnit);

	int oldHp = hp;
	hp -= dmg;

	trigger_health_threshold_augments();
	owner.try_knockback(oldHp, attackerUnit.combat.hitIndex, attackerUnit.stats);

	return hp <= 0 && !try_proc_survive();
}
bool UnitStatus::take_damage(const Surge& surge) {
	int dmg = surge.calculate_damage_taken_and_apply_augments(owner);

	int oldHp = hp;
	hp -= dmg;

	trigger_health_threshold_augments();
	owner.try_knockback(oldHp, surge.hitIndex, surge.stats);

	return hp <= 0 && !try_proc_survive();
}
bool UnitStatus::take_damage(int dmg, bool shove) {
	dmg *= get_corrosion_multiplier();
	dmg *= get_reinforcement_multiplier();

	int oldHp = hp;
	hp -= dmg;

	trigger_health_threshold_augments();

	if (!owner.anim.in_knockback() && met_knockback_threshold(oldHp)) 
		owner.movement.knockback();
	else if (shove) owner.movement.knockback(0.5f);

	return hp <= 0 && !try_proc_survive();
}
#pragma endregion


bool UnitStatus::met_knockback_threshold(int oldHp) {
	bool metThreshold = false;

	for (int i = kbIndex; i <= owner.stats->knockbacks; i++) {
		int threshold = owner.stats->maxHp - (owner.stats->maxHp * i / owner.stats->knockbacks);

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
bool UnitStatus::try_proc_survive() {
	if (owner.stats->try_proc_augment(AugmentType::SURVIVOR)) {
		remove(statusFlags, AugmentType::SURVIVOR);

		hp = 1;
		kbIndex = owner.stats->knockbacks;
		std::cout << "SURVIVED BITCH" << std::endl;

		return true;
	}
	return false;
}
void UnitStatus::syphon(const Augment* syphon, bool fromLinking) {
	int newKbIndex = std::max(kbIndex - (int)syphon->data.syphon.restoredKnockbacks, 1);
	int oldHPThreshold = owner.stats->maxHp - (owner.stats->maxHp * newKbIndex / owner.stats->knockbacks);

	hp = oldHPThreshold + 1;
	kbIndex = newKbIndex;

	// LINK the healing to ally units if this Unit have it 
	if (!fromLinking && owner.stats->has_augment(AugmentType::LINK))
		link_augment(*syphon);
}
