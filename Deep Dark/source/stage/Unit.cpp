#include "Unit.h"
#include "Stage.h"
#include <iostream>

const float KNOCKBACK_FORCE = 50.0f;
const float KNOCKBACK_DURATION = 1.4f;
const float FALL_DURATION = 1.25f;
const float SQUASH_DURATION = 1.3f;
const float LAUNCHING_DURATION = .65f;
const float DROPPING_DURATION = .7f;
const float LAUNCH_FORCE = 75.f;

const float RUST_TYPE_LEDGE_RANGE = 15.f;
const float LEDGE_SNAP = 25.0f;
const float JUMP_DURATION = 1.f;
const float LEAP_DURATION = 0.7f;

const float BASE_PHASING_TIMER = 1.f; // over 50 distance
const float BASE_PHASE_DISTANCE = 50;

const float STATUS_ICON_SPACING = 40.f;

Unit::Unit(Stage* curStage, sf::Vector2f startPos, int startingLane, const UnitStats* data,
	UnitAniMap* p_aniMap, int id)
	: sprite(get_default_texture()), marker({ 20.f,40.f }), currentLane(startingLane), stage(curStage),
	aniMap(p_aniMap), stats(data), id(id)
{
	activeStatuses.reserve(MAX_EFFECTS);
	marker.setFillColor(sf::Color::Cyan);
	pos = startPos;
	hp = data->maxHp;
	kbIndex = 1;
	hitIndex = 0;
	attackCooldown = 0.0f;

	//tries to get augment to flip status mask. If it doesnt have the Augment,
	// it will give an empty Augment with NONE, or 0
	statuses = 0;
	shieldHp = (int)stats->get_augment(SHIELD).value; 
	statuses |= has_augment(SURVIVOR) ? SURVIVOR : 0;
	statuses |= has_augment(PHASE) ? PHASE : 0;

	marker.setOrigin(marker.getSize());

	start_animation(UnitAnimationState::MOVING);
 	//std::cout << "new Unit has been created, TEAM: " << data->team << std::endl;
}
Unit::Unit(Stage* stage, Surge& surge) :
	Unit(stage, surge.pos, surge.currentLane, surge.stats, nullptr, -2) { }

RequestType Unit::update_tween(float deltaTime) {
	pos = tween->update_and_get(deltaTime);
	if (tween->isComplete) {
		RequestType type = tween->tweenType;
		cancel_tween();
		return type;
	}
	return RequestType::NOT_DONE;
}
void Unit::create_tween(sf::Vector2f endPos, float time,
	RequestType tweenType, bool overwrite) {
	if (tweening())
		if (overwrite) tween = nullptr;
		else return;

	tween = std::make_unique<UnitTween>(pos, endPos, time, tweenType);
	//std::cout << "created tween" << std::endl;
}
void Unit::destroy_unit() {
	std::cout << "destroying Unit with id #" << id << std::endl;
	cancel_tween();
	stage->recorder->add_death(stats->team, currentLane, causeOfDeath);
	if (is_summoned_unit()) stage->summonData->count--;

	hp = 0;
	animationState = UnitAnimationState::DYING;
}
bool Unit::move_req_check() { return !stage->can_push_move_request(id); }
float roll_for_status() {
	static std::minstd_rand gen(std::random_device{}());
	std::uniform_real_distribution<float> dis(0, 100);
	return dis(gen);
}
bool over_this_gap(std::pair<float, float> gap, float xPos) {
	return xPos > gap.first && xPos < gap.second;
}
float calculate_phase_timer(float distance) {
	return (distance / BASE_PHASE_DISTANCE) * BASE_PHASING_TIMER;
}

// Drawing and Animating
void Unit::start_animation(UnitAnimationState newState) {
	animationState = newState;
	(*aniMap)[newState].reset(aniTime, currentFrame, sprite);
}
void Unit::start_idle_or_attack_animation()  {
	if (attackCooldown <= 0) 
		start_animation(UnitAnimationState::ATTACKING);
	else start_animation(UnitAnimationState::IDLING);
}
int Unit::update_animation(float deltaTime) {
	int events = (*aniMap)[animationState].update(aniTime, currentFrame, deltaTime, sprite);
	sprite.setPosition(pos);
	marker.setPosition(pos);

	return events;
}

// Movement
void Unit::move(float deltaTime) {
	float speed = slowed() ? 0.1f : stats->speed;
	pos.x += speed * deltaTime * stats->team;
}
void Unit::knockback(float force) {
	if (has_augment(LIGHTWEIGHT)) force *= 1.5f;
	if (has_augment(HEAVYWEIGHT)) force *= 0.7f;

	float newX = pos.x - (KNOCKBACK_FORCE * force * stats->team);
	auto [minWall, maxWall] = stage->get_walls(currentLane);
	newX = std::clamp(newX, minWall, maxWall);

	sf::Vector2f newPos({ newX, stage->get_lane(currentLane).yPos});

	force = std::min(force, 1.2f);
	create_tween(newPos, KNOCKBACK_DURATION * force, RequestType::KNOCKBACK);
	if (tween) tween->easingFuncX = EasingType::OUT_CUBIC;

	start_animation(UnitAnimationState::KNOCKEDBACK);
}
void Unit::fall(float newY) {
	sf::Vector2f newPos({ pos.x, newY });

	create_tween(newPos, FALL_DURATION, RequestType::FALL);
	if (tween) tween->easingFuncY = EasingType::IN_OUT_SINE;

	start_animation(UnitAnimationState::FALLING);
}
void Unit::squash(float newY) {
	sf::Vector2f newPos({ pos.x, newY });

	create_tween(newPos, SQUASH_DURATION, RequestType::SQUASH);
	if (tween) tween->easingFuncY = EasingType::OUT_BOUNCE;

	start_animation(UnitAnimationState::KNOCKEDBACK);
}
void Unit::launch(float newY) {
//	std::cout << "starting launch" << std::endl;
	sf::Vector2f newPos({ pos.x, newY - LAUNCH_FORCE });

	create_tween(newPos, LAUNCHING_DURATION,RequestType::LAUNCH);
	if (tween) tween->easingFuncY = EasingType::OUT_QUART;

	start_animation(UnitAnimationState::KNOCKEDBACK);
}
void Unit::teleport() {
	Teleporter* tp = stage->lanes[currentLane].get_teleporter(stats->team);
	if (!tp || !stage->can_push_move_request(id)) return;

	start_animation(UnitAnimationState::MOVING);
	std::cout << "FOUND teleporter" << std::endl;
	if (tp->connectedLane != currentLane)
		stage->push_move_request(id, currentLane, tp->connectedLane, stats->team, tp->xDestination, RequestType::TELEPORT);
	else pos = { tp->xDestination, stage->lanes[tp->connectedLane].yPos };
}
void Unit::jump(float newX) {
	sf::Vector2f newPos({ newX, stage->get_lane(currentLane).yPos });

	create_tween(newPos, JUMP_DURATION, RequestType::JUMP);
	if (tween) {
		tween->easingFuncX = EasingType::LINEAR;
		tween->easingFuncY = EasingType::OUT_BACK;
	}

	start_animation(UnitAnimationState::JUMPING);
}
bool Unit::try_leap() {
	for (auto& gap : stage->get_lane(currentLane).gaps) {
		float leapRange = stats->get_augment(LEAP).value;
		float edge = player_team() ? gap.second : gap.first;
		float dist = (edge - pos.x) * stats->team;

		if (dist <= 0 || dist > leapRange || over_this_gap(gap, pos.x)) continue;

		float landingSpot = edge + LEDGE_SNAP * stats->team;
		sf::Vector2f newPos({ landingSpot, pos.y });

		create_tween(newPos, LEAP_DURATION, RequestType::LEAP);
		if (tween) tween->easingFuncX = EasingType::LINEAR;

		start_animation(UnitAnimationState::JUMPING);
		return true;
	}

	return false;
}
void Unit::warp(const UnitStats* enemyStats) {
	Augment aug = enemyStats->get_augment(WARP);

	currentLane = std::clamp(currentLane + aug.intValue, 0, stage->laneCount - 1);

	float newX = pos.x - (aug.value * stats->team);
	auto [minWall, maxWall] = stage->get_walls(currentLane);
	pos.x = std::clamp(newX, minWall, maxWall);

	attackCooldown = aug.value2;
}

void Unit::try_knockback(int oldHp, int hitIndex, const UnitStats* enemyStats) {
	if (in_knockback()) return;
	if (met_knockback_threshold(oldHp, hp)) {
		shieldHp = (int)stats->get_augment(SHIELD).intValue;
		float force = enemyStats->has_augment(BULLY) ? 1.5f : 1;

		if (enemyStats->has_augment(AugmentType::SQUASH))
			push_squash_request();
		else if (enemyStats->has_augment(AugmentType::LAUNCH))
			push_launch_request();
		else
			knockback(force);
	}
	else if (trigger_augment(enemyStats, SHOVE, hitIndex)) 
		knockback(0.5f);
	else if (trigger_augment(enemyStats, WARP, hitIndex)) 
		warp(enemyStats);
}
void Unit::push_fall_request() {
	std::pair<float, int> fallTo = stage->find_lane_to_fall_on(*this);
	// will set hp to -100 if no lane is found (aka, they fell through ALL lanes into the void)

	if (hp < 0) fall(fallTo.first);
	else
		stage->push_move_request(*this, fallTo.second, fallTo.first, RequestType::FALL);
}
void Unit::push_squash_request() {
	if (currentLane == 0) {
		knockback(false);
		return;
	}

	int lane = stage->find_lane_to_knock_to(*this, -1);

	if (lane == currentLane) knockback(false);
	else
		stage->push_move_request(*this, lane, stage->get_lane(lane).yPos, RequestType::SQUASH);
}
void Unit::push_launch_request() {
	if (currentLane == stage->laneCount - 1 || has_augment(HEAVYWEIGHT)) {
		knockback(false);
		return;
	}

	int lane = stage->find_lane_to_knock_to(*this, 1);
	std::cout << "Found Lane to LAUNCH to, its Lane #" << lane << std::endl;
	if (lane == currentLane) knockback(false);
	else
		stage->push_move_request(*this, lane, stage->get_lane(lane).yPos, RequestType::LAUNCH);
}
bool Unit::try_push_jump_request() {
	int targetLane = currentLane + 1;
	if (stage->out_of_lane(targetLane, pos.x)) return false;
	float jumpRange = stats->get_augment(JUMP).value;
	Lane& tarLane = stage->get_lane(targetLane);

	for (auto& gap : tarLane.gaps) {
		if (!over_this_gap(gap, pos.x)) continue; // only jump on overhead gaps your over
		float edge = player_team() ? gap.second : gap.first;
		float dist = (edge - pos.x) * stats->team;

		if (dist <= 0 || dist > jumpRange) return false;
		// I have to make sure gaps aren't too close to each other.
		float landingSpot = edge + LEDGE_SNAP * stats->team;
		stage->push_move_request(*this, targetLane, landingSpot, RequestType::JUMP);
		return true;
	}
	return false;
}
void Unit::finish_launch_tween() {
	sf::Vector2f newPos = { pos.x, stage->get_lane(currentLane).yPos };
	create_tween(newPos, DROPPING_DURATION, RequestType::NONE);
	if (tween) tween->easingFuncY = EasingType::OUT_BOUNCE;
}

// Checks
bool Unit::met_knockback_threshold(int oldHp, int newHp) {
	bool metThreshold = false;

	for (int i = kbIndex; i <= stats->knockbacks; i++) {
		int threshold = stats->maxHp - (stats->maxHp * i / stats->knockbacks);

		if (oldHp > threshold && newHp <= threshold) {
			//std::cout << "MET KB THRESHOLD #" << kbIndex << " of hp: " << threshold 
				//<<" ----- maxHp: " << stats->maxHp << " - oldHp: " << oldHp << " - newHp: " << newHp << std::endl;
			metThreshold = true;
			kbIndex = i + 1;
		}
		else break;
	}

	return metThreshold;
}
bool Unit::base_in_range(float sightDist) {
	return enemy_in_range(stage->get_enemy_base(stats->team).pos.x, 0, sightDist);
}
bool Unit::enemy_in_range(float xPos, float minRange, float maxRange) {
	float dist = (xPos - pos.x) * stats->team;
	return dist >= minRange && dist <= maxRange;
}
bool Unit::enemy_is_in_sight_range() {
	float sightMultiplier = blinded() ? 0.5f : 1;
	float sightDist = stats->sightRange * sightMultiplier;

	if (base_in_range(sightDist))
		return true;

	auto [minLane, maxLane] = get_lane_sight_range();
	
	for (int i = minLane; i < maxLane; i++) {
		std::vector<Unit>& enemyUnits = stage->get_lane_targets(i, stats->team);
		for (auto it = enemyUnits.begin(); it != enemyUnits.end(); ++it) {
			if (is_valid_target(*it, 0, sightDist)) return true;
		}
	}

	return false;
}
bool Unit::is_valid_target(const Unit& enemy, float minRange, float maxRange) {
	return !enemy.invincible() && !enemy.pending_death() &&
		enemy_in_range(enemy.pos.x, minRange, maxRange);
}
bool Unit::can_teleport() const { return !ancient_type() && stage->can_teleport(pos, currentLane, stats->team); }
bool Unit::over_gap() const { return stage->over_gap(currentLane, pos.x); }
bool Unit::try_proc_augment(const std::vector<Augment>& augments, AugmentType targetAugment, int hit) {
	for (auto& augment : augments) {
		if (augment.augType != targetAugment) continue;
		return augment.can_hit(hit) && roll_for_status() <= augment.percentage;
	}

	return false;
}
bool Unit::try_proc_survive() {
	if (statuses & SURVIVOR && trigger_augment(stats, SURVIVOR)) {
		statuses &= ~SURVIVOR;
		hp = 1;
		kbIndex = stats->knockbacks;
		std::cout << "SURVIVED BITCH" << std::endl;
		return true;
	}
	return false;
}
bool Unit::can_make_surge(const Augment& aug) {
	return aug.is_surge() && roll_for_status() <= aug.percentage &&
		aug.can_hit(hitIndex);
}
bool Unit::rust_type_and_near_gap() {
	if (!rusted_tyoe()) return false;

	for (auto& gap : stage->get_lane(currentLane).gaps) {
		float edge = player_team() ? gap.first : gap.second;
		float dist = (edge - pos.x) * stats->team;

		if (dist > 0 || dist <= RUST_TYPE_LEDGE_RANGE)
			return true;
	}
	return false;
}
bool Unit::try_terminate_unit(Unit& enemyUnit, int dmg) {
	if (!has_augment(TERMINATE)) return false;
	float threshold = stats->get_augment(TERMINATE).value;
	float curHpPercent = (float)(enemyUnit.hp - dmg) / enemyUnit.stats->maxHp;

	return curHpPercent <= threshold;
}

// Combat
void Unit::counter_surge(AugmentType& surgeType) {
	const Augment& augment = stats->get_augment(COUNTER_SURGE);
	float distance = augment.value;
	Augment newSurge = Augment::surge(surgeType, distance, augment.surgeLevel);
	stage->create_surge(*this, newSurge);
}
void Unit::on_kill(Unit& enemyUnit) {
	if (trigger_augment(stats, PLUNDER)) enemyUnit.statuses |= PLUNDER;
	if (stats->has_augment(CODE_BREAKER)) enemyUnit.statuses |= CODE_BREAKER;
	if (trigger_augment(stats, SALVAGE)) stage->create_summon(*this);
	enemyUnit.causeOfDeath = DeathCause::UNIT;
}
float Unit::calculate_damage_reduction(const std::vector<Augment>& augments) {
	// called from attacked Unit, parameters are from the attackING unit.
	float boost = 1;

	for (const Augment& augment : augments) {
		auto aug = augment.augType;
		if (!augment.is_damage_modifier()) continue;

		if (aug & AugmentType::RESIST)
			boost *= augment.percentage;
		else if (aug & AugmentType::SUPERIOR)
			boost /= augment.percentage;
	}

	return boost;
}
void Unit::add_status_effect(const Augment& aug) {
	// Remove existing effect of same type (so new slow overrides old)
	if (!aug.is_status_effect()) {
		std::cout << "Augment type was not a status. Cannot add effect" << std::endl;
		return;
	}

	StatusEffect newEffect(aug.augType, aug.value);
	std::cout << "adding effect, bit mask value: " << aug.augType << std::endl;

	activeStatuses.erase(
		std::remove_if(activeStatuses.begin(), activeStatuses.end(),
			[&](const StatusEffect& existing) {
				if (existing.effect == newEffect.effect) {
					statuses &= ~existing.effect;
					return true;
				}
				return false;
			}),
		activeStatuses.end()
	);

	if (activeStatuses.size() >= MAX_EFFECTS) {
		if (newEffect.effect & AugmentType::STRENGTHEN) {
			statuses &= ~activeStatuses.back().effect;
			activeStatuses.pop_back();
			statuses |= AugmentType::STRENGTHEN;
			activeStatuses.push_back(newEffect);
		}
		else return;
	}

	statuses |= newEffect.effect;
	activeStatuses.push_back(newEffect);
}
void Unit::update_status_effects(float deltaTime) {
	if (activeStatuses.size() <= 0) return;

	sf::Vector2f statusPos = { pos.x, pos.y - 20.f };

	for (size_t i = activeStatuses.size(); i--;) {
		auto& status = activeStatuses[i];
		status.update(deltaTime);

		if (status.is_expired()) {
			statuses &= ~status.effect;
			activeStatuses[i] = std::move(activeStatuses.back());
			activeStatuses.pop_back();
		}
		else {
			stage->effectSpritePositions.emplace_back(status.effect, statusPos);
			statusPos.x -= STATUS_ICON_SPACING * stats->team;
		}
	}
}
int Unit::apply_effects(const std::vector<Augment>& augments, int hitIndex, int dmg) {
	// called from attacked Unit, parameters are from the attackING unit.
	for (const Augment& augment : augments) {
		if (can_proc_status(augment, hitIndex)) {
			if (roll_for_status() < augment.percentage) {
				add_status_effect(augment);
			}
		}
		else if (augment.is_damage_modifier())
			dmg = (int)(dmg * augment.percentage);
		else if (augment.augType & CRITICAL && roll_for_status() < augment.percentage)
			dmg *= 2;
	}
	
	return dmg;
}

int Unit::calculate_damage_and_effects(Unit& attackingUnit) {
	// called from attacked Unit, parameters are from the attackING unit.
	// Run calculations from ABILITIES and DEBUFFS
	int dmg = attackingUnit.get_dmg();
	dmg = corroded() ? (int)(dmg * 2.f) : dmg;
	dmg = attackingUnit.weakened() ? (int)(dmg * .5f) : dmg;

	if (targeted_by_unit(attackingUnit))
		dmg = apply_effects(attackingUnit.stats->augments, attackingUnit.hitIndex, dmg);

	if (attackingUnit.targeted_by_unit(*this))
		dmg = (int)(dmg * calculate_damage_reduction(stats->augments));
	
	// Try break the Shield BEFORE void and Terminate, as they do NOT go through shields
	if (has_shield_up() && !damage_shield(dmg, attackingUnit.stats)) return 0; // return if shield did not break

	if (targeted_by_unit(attackingUnit)) {
		if (trigger_augment(attackingUnit.stats, VOID, attackingUnit.hitIndex))
			dmg += (int)(stats->maxHp * attackingUnit.stats->get_augment(VOID).value);
		if (attackingUnit.try_terminate_unit(*this, dmg))
			dmg += stats->maxHp;
	}

	return dmg;
}
bool Unit::damage_shield(int& dmg, const UnitStats* _stats) {
	if (_stats && trigger_augment(_stats, SHIELD_PIERCE, hitIndex)) {
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
bool Unit::take_damage(Unit& attackingUnit) {
	int dmg = calculate_damage_and_effects(attackingUnit);

	int oldHp = hp;
	hp -= dmg;
	//std::cout << "ID: " << id << " - hit, oldHp: " << oldHp << " - newHp: " << hp << " - dmg taken: " << dmg << std::endl;

	try_knockback(oldHp, attackingUnit.hitIndex, attackingUnit.stats);
	return hp <= 0 && !try_proc_survive();
}
bool Unit::take_damage(Surge& surge) {
	if (has_augment(COUNTER_SURGE) && surge.never_hit_unit(id))
		counter_surge(surge.surgeType);

	int dmg = surge.get_dmg();
	dmg = corroded() ? (int)(dmg * 2.f) : dmg;

	if (targeted_by_unit(surge.stats->targetTypes))
		dmg = apply_effects(surge.stats->augments, surge.hitIndex, dmg);
	if (surge.targeted_by_unit(stats->targetTypes))
		dmg = (int)(dmg * calculate_damage_reduction(stats->augments));

	if (has_shield_up() && !damage_shield(dmg, surge.stats)) return false; // return if shield did not BROKEN
	if (targeted_by_unit(surge.stats->targetTypes)) {
		if (trigger_augment(surge.stats, VOID, surge.hitIndex))
			dmg += (int)(stats->maxHp * surge.stats->get_augment(VOID).value);
		if (surge.try_terminate_unit(*this, dmg))
			dmg += stats->maxHp;
	}

	int oldHp = hp;
	hp -= dmg;
	//std::cout << "ID: " << id << " - hit, by a SURGE, oldHp: " << oldHp << " - dmg taken: " << dmg << " - newHp: " << hp << std::endl;

	try_knockback(oldHp, surge.hitIndex, surge.stats);

	return hp <= 0 && !try_proc_survive();
}
bool Unit::take_damage(int dmg, bool shove) {
	dmg = corroded() ? (int)(dmg * 2.f) : dmg;
	if (has_shield_up() && !damage_shield(dmg)) return false; // return if shield did not break

	int oldHp = hp;
	hp -= dmg;

	if (!in_knockback() && met_knockback_threshold(oldHp, hp)){
		shieldHp = (int)stats->get_augment(SHIELD).value;
		knockback();
	} 
	else if (shove) knockback(0.5f);

	return hp <= 0 && !try_proc_survive();
}

/*
void Unit::attack() {
	attackCooldown = stats->attackTime;

	auto [minLane, maxLane] = get_lane_reach();
	auto [minAttackRange, maxAttackRange] = get_attack_range();
	bool hitEnemy = false;

	for (int i = minLane; i < maxLane; i++) {
		std::vector<Unit>& enemyUnits = stage->get_lane_targets(i, stats->team);
		float minDist = abs(pos.x - stage->get_enemy_base(stats->team).pos.x);
		Unit* singleTargetedUnit = nullptr;

		for (auto it = enemyUnits.begin(); it != enemyUnits.end(); ++it) {
			if (is_valid_target(*it, minAttackRange, maxAttackRange)) {
				if (!stats->singleTarget) {
					hitEnemy = true;
					if (it->take_damage(*this)) on_kill(*it);
				}
				else {
					float dist = abs(pos.x - it->pos.x);
					if (dist < minDist) {
						hitEnemy = true;
						minDist = dist;
						singleTargetedUnit = &(*it);;
					}
				}
			}
		}

		if (singleTargetedUnit && stats->singleTarget) 
			if (singleTargetedUnit->take_damage(*this)) on_kill(*singleTargetedUnit);
	}

	try_attack_enemy_base(hitEnemy);
	try_create_surge(hitEnemy);
	try_create_projectile();

	hitIndex = (hitIndex + 1) % stats->totalHits;
}
*/
// Attackign Functions
void Unit::attack() {
	attackCooldown = stats->attackTime;
	stage->create_hitbox_visualizers(pos, get_attack_range(), stats->team);

	bool hitEnemy = process_attack_on_lanes();
	handle_post_attack_effects(hitEnemy);
	hitIndex = (hitIndex + 1) % stats->totalHits;
}
bool Unit::process_attack_on_lanes() {
	auto [minLane, maxLane] = get_lane_reach();
	bool hitEnemy = false;
	for (int lane = minLane; lane < maxLane; lane++) {
		hitEnemy |= attack_lane(lane);
	}
	return hitEnemy;
}
bool Unit::attack_lane(int laneIndex) {
	std::vector<Unit>& enemies = stage->get_lane_targets(laneIndex, stats->team);

	if (stats->singleTarget)
		return attack_single_target(enemies);
	else
		return attack_all_targets(enemies);
}
bool Unit::attack_single_target(std::vector<Unit>& enemies) {
	Unit* singleTargetedUnit = nullptr;
	auto [minAttackRange, maxAttackRange] = get_attack_range();
	float minDist = abs(pos.x - stage->get_enemy_base(stats->team).pos.x);

	for (auto it = enemies.begin(); it != enemies.end(); ++it) {
		if (is_valid_target(*it, minAttackRange, maxAttackRange)) {
			float dist = abs(pos.x - it->pos.x);
			if (dist < minDist) {
				minDist = dist;
				singleTargetedUnit = &(*it);;
			}
		}
	}

	if (singleTargetedUnit && stats->singleTarget)
		if (singleTargetedUnit->take_damage(*this)) {
			on_kill(*singleTargetedUnit);
			return true;
		}

	return false;
}
bool Unit::attack_all_targets(std::vector<Unit>& enemies) {
	bool hitEnemy = false;
	auto [minAttackRange, maxAttackRange] = get_attack_range();
	for (auto it = enemies.begin(); it != enemies.end(); ++it) {
		if (is_valid_target(*it, minAttackRange, maxAttackRange)) {
			hitEnemy = true;
			if (it->take_damage(*this)) on_kill(*it);
		}
	}

	return hitEnemy;
}
void Unit::handle_post_attack_effects(bool hitEnemy) {
	try_attack_enemy_base(hitEnemy);
	try_create_surge(hitEnemy);
	try_create_projectile();
}
void Unit::try_create_surge(bool hitEnemy) {
	if (!hitEnemy || !stats->has_surge()) return;
		
	for (auto& augment : stats->augments) {
		if (can_make_surge(augment)) {
			stage->create_surge(*this, augment);
			if (augment.augType != ORBITAL_STRIKE) continue;

			// Create additional orbital strikes with spacing
			int additionalStrikes = augment.surgeLevel - 1;
			float strikeSpacing = augment.value2;
			float currentOffset = strikeSpacing;
			for (int i = 0; i < additionalStrikes; i++) {
				Surge* strike = stage->create_surge(*this, augment);
				strike->pos.x += currentOffset;
				currentOffset += strikeSpacing;
			}
		}
	}
}
void Unit::try_attack_enemy_base(bool& hitEnemy) {
	if (!stats->singleTarget || (stats->singleTarget && !hitEnemy)) {
		Base& enemyBase = stage->get_enemy_base(stats->team);
		auto [minRange, maxRange] = get_attack_range();
		
		if (enemy_in_range(enemyBase.pos.x, minRange, maxRange)) {
			enemyBase.take_damage(get_dmg());
			hitEnemy = true;
		}
	}
}
void Unit::try_create_projectile() {
	if (!has_augment(PROJECTILE)) return;

	for (auto& aug : stats->augments) 
		if (aug.augType & PROJECTILE && aug.can_hit(hitIndex))
			stage->create_projectile(*this, aug);
}

// Tick
void Unit::tick(float deltaTime) {
	if (overloaded()) deltaTime *= 0.5f;

	attackCooldown -= deltaTime;
	update_status_effects(deltaTime);

	switch (animationState) {
	case UnitAnimationState::MOVING:
		moving_state(deltaTime);
		break;
	case UnitAnimationState::ATTACKING:
		attack_state(deltaTime);
		break;
	case UnitAnimationState::IDLING:
		idling_state(deltaTime);
		break;
	case UnitAnimationState::KNOCKEDBACK:
		knockback_state(deltaTime);
		break;
	case UnitAnimationState::FALLING:
		falling_state(deltaTime);
		break;
	case UnitAnimationState::JUMPING:
		jumping_state(deltaTime);
		break;
	case UnitAnimationState::PHASE:
		phase_state(deltaTime);
		break;
	default:
		waiting_state(deltaTime);
		break;
	}
}
void Unit::moving_state(float deltaTime) {
	update_animation(deltaTime);
	move(deltaTime);

	if (can_teleport())
		teleport();
	else if (can_fall())
		push_fall_request();
	else if (enemy_is_in_sight_range()) {
		//std::cout << "found enemy" << std::endl;
		if (can_phase()) start_animation(UnitAnimationState::PHASE);
		else start_idle_or_attack_animation();
	}
	else {
		// in case Unit has both JUMP and LEAP, return if they succeed in jumping
		if (has_augment(JUMP) && currentLane < stage->laneCount - 1)
			if (try_push_jump_request()) return; 
		if (has_augment(LEAP)) 
			if (try_leap()) return;
		if (rust_type_and_near_gap())
			start_animation(UnitAnimationState::IDLING);
	}
}
void Unit::attack_state(float deltaTime) {
	int events = update_animation(deltaTime);
	if (Animation::check_for_event(AnimationEvent::FIRST_FRAME, events))
		hitIndex = 0;
	if (Animation::check_for_event(AnimationEvent::FINAL_FRAME, events)) {
		attackCooldown = stats->attackTime;
		if (has_augment(SELF_DESTRUCT)) destroy_unit();
		else if (enemy_is_in_sight_range()) 
			start_idle_or_attack_animation();	
		else if (!rusted_tyoe() || (rusted_tyoe() && !can_fall()))
			start_animation(UnitAnimationState::MOVING);
		else
			start_animation(UnitAnimationState::IDLING);
	}
	if (Animation::check_for_event(AnimationEvent::ATTACK, events)) {
		attack();
		if (has_augment(SELF_DESTRUCT)) hp = 0;
	}
}
void Unit::idling_state(float deltaTime) {
	update_animation(deltaTime);
	if (enemy_is_in_sight_range())
		start_idle_or_attack_animation();
	else if (!rust_type_and_near_gap())
		start_animation(UnitAnimationState::MOVING);
}
void Unit::knockback_state(float deltaTime) {
	update_animation(deltaTime);
	if (can_teleport())
		teleport();
	else if (can_fall()) 
		push_fall_request();
	else if (!tweening()) {
		//std::cout << "after knockback: at Y-" << pos.y << "AND on lane #" << currentLane << std::endl;
		if (hp <= 0) destroy_unit();
		else if (enemy_is_in_sight_range())
			start_idle_or_attack_animation();
		else start_animation(UnitAnimationState::MOVING);
	}
	else {
		RequestType finishedType = update_tween(deltaTime);
		if (finishedType == RequestType::LAUNCH) finish_launch_tween();
	}
}
void Unit::falling_state(float deltaTime) {
	int events = update_animation(deltaTime);

	if (done_tweening_and_animating(events)) {
		if (hp <= 0) destroy_unit();
		else if (enemy_is_in_sight_range())
			start_idle_or_attack_animation();
		else start_animation(UnitAnimationState::MOVING);
	}
	else update_tween(deltaTime);
}
void Unit::jumping_state(float deltaTime) {
	update_animation(deltaTime);

	if (!tweening()) {
		if (hp <= 0) destroy_unit();
		else if (enemy_is_in_sight_range())
			start_idle_or_attack_animation();
		else start_animation(UnitAnimationState::MOVING);
	}
	else update_tween(deltaTime);
}
void Unit::phase_state(float deltaTime) {
	auto events = update_animation(deltaTime);

	if (Animation::check_for_event(AnimationEvent::FINAL_FRAME, events)) {
		if (can_phase()) { // if can still phase
			statuses &= ~PHASE;
			float end = pos.x + stats->get_augment(PHASE).value * stats->team;
			auto [minWall, maxWall] = stage->get_walls(currentLane);
			end = std::clamp(end, minWall, maxWall);
			float timer = calculate_phase_timer(std::abs(pos.x - end));

			pos.x = end, pos.y;
			attackCooldown = timer;
			animationState = UnitAnimationState::IS_PHASING;
		}
		else {
			if (hp <= 0) destroy_unit();
			else if (enemy_is_in_sight_range())
				start_idle_or_attack_animation();
			else start_animation(UnitAnimationState::MOVING);
		}
	}
}
void Unit::waiting_state(float deltaTime) {
	if (attackCooldown <= 0) {
		if (animationState == UnitAnimationState::IS_PHASING)
			start_animation(UnitAnimationState::PHASE);
		else {
			if (enemy_is_in_sight_range())
				start_idle_or_attack_animation();
			else
				start_animation(UnitAnimationState::MOVING);
		}
	}
}

std::pair<int, int> Unit::get_lane_reach() {
	auto& hit = stats->get_hit_stats(hitIndex);
	// laneReach.first will be negative
	int min = std::max(currentLane - std::abs(hit.laneReach.first), 0);
	int max = std::min(currentLane + hit.laneReach.second + 1, stage->laneCount);
	return { min, max };
}
std::pair<int, int> Unit::get_lane_sight_range() const {
	int min = std::max(currentLane - stats->laneSight.first, 0);
	int max = std::min(currentLane + stats->laneSight.second + 1, stage->laneCount);
	return { min, max };
}
//	std::cout << "ID: " << id << " - hit, by UNIT, oldHp: " << oldHp << " - dmg taken : " << dmg << " - newHp : " << hp << std::endl;