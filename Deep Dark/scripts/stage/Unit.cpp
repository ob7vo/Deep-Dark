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
const float LEDGE_SNAP = 25.0f;
const float JUMP_DURATION = 1.f;
const float LEAP_DURATION = 0.7f;
const float BASE_PHASING_TIMER = 1.f; // over 50 distance
const float BASE_PHASE_DISTANCE = 50;
const int MAX_EFFECTS = 4;

Unit::Unit(Stage* curStage, sf::Vector2f startPos, int startingLane, const UnitStats* data,
	std::array<Animation, 5>* p_aniMap, int id)
	: sprite(get_default_texture()), marker({ 5.f,5.f }), currentLane(startingLane), stage(curStage),
	aniMap(p_aniMap), stats(data), id(id)
{
	activeStatuses.reserve(MAX_EFFECTS);
	marker.setFillColor(sf::Color::Cyan);
	pos = startPos;
	hp = data->maxHp;
	kbIndex = 1;
	attackCooldownTimer = 0.0f;

	//tries to get augment to flip status mask. If it doesnt have the Augment,
	// it will give an empty Augment with NONE, or 0
	statuses = 0;
	shieldHp = (int)stats->get_augment(SHIELD).value; 
	statuses |= has_augment(SURVIVOR) ? SURVIVOR : 0;
	statuses |= has_augment(PHASE) ? PHASE : 0;

	marker.setOrigin({ 1,1 });

	start_animation(UnitAnimationState::MOVING);
 	std::cout << "new Unit has been created, TEAM: " << data->team << std::endl;
}
void Unit::destroy_unit() {
	std::cout << "destroying Unit with id #" << id << std::endl;
	cancel_tweens();
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
	(*aniMap)[static_cast<int>(newState)].start(aniTime, currentFrame, sprite);
}
void Unit::start_special_animation(UnitAnimationState specialState) {
	if (!stats->specialAnimation) return;

	animationState = specialState;
	stats->specialAnimation->start(aniTime, currentFrame, sprite);
}
void Unit::start_idle_or_attack_animation()  {
	if (attackCooldownTimer <= 0) {
		attackCooldownTimer = stats->attackTime;
		start_animation(UnitAnimationState::ATTACKING);
	}
	else start_animation(UnitAnimationState::IDILING);
}
std::vector<AnimationEvent> Unit::draw(sf::RenderWindow& window, float deltaTime) {
	auto events = (*aniMap)[static_cast<int>(animationState)].update(aniTime, currentFrame, deltaTime, sprite);
	sprite.setPosition(pos);
	marker.setPosition(pos);
	window.draw(sprite);
	window.draw(marker);

	return events;
}
std::vector<AnimationEvent> Unit::draw_special(sf::RenderWindow& window, float deltaTime) {
	if (!stats->specialAnimation) {
		std::cout << "no special animation is availible" << std::endl;
		return {};
	}

	auto events = stats->specialAnimation->update(aniTime, currentFrame, deltaTime, sprite);
	sprite.setPosition(pos);
	marker.setPosition(pos);
	window.draw(sprite);
	window.draw(marker);

	return events;
}

// Movement
void Unit::move(float deltaTime) {
	float speed = slowed() ? 0.1f : stats->speed;
	pos.x += speed * deltaTime * stats->team;
}
void Unit::knockback(bool shove) {
	float force = shove ? 0.5f : 1.f;
	float newX = pos.x - (KNOCKBACK_FORCE * force * stats->team);
	auto [minWall, maxWall] = stage->get_walls(currentLane);
	newX = std::clamp(newX, minWall, maxWall);

	sf::Vector2f newPos({ newX, stage->get_lane(currentLane).yPos});
	Tween::move(&pos, newPos, KNOCKBACK_DURATION * force).setEase(Easing::easeOutCubic);

	start_animation(UnitAnimationState::KNOCKEDBACK);
}
void Unit::fall(float newY) {
	sf::Vector2f newPos({ pos.x, newY });
	//pos = newPos;
	Tween::move(&pos, newPos, FALL_DURATION, true).setEase(Easing::easeInOutSine);
	start_animation(UnitAnimationState::FALLING);
}
void Unit::squash(float newY) {
	Tween::move(&pos.y, newY, SQUASH_DURATION).setEase(Easing::easeOutBounce);
	start_animation(UnitAnimationState::KNOCKEDBACK);
}
void Unit::launch(float newY) {
	Tween::move(&pos.y, newY - LAUNCH_FORCE, LAUNCHING_DURATION).setEase(Easing::easeOutQuart).setOnComplete(
		[=]() {
		Tween::move(&pos.y, newY, DROPPING_DURATION, true).setEase(Easing::easeOutBounce);
		});
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
	start_special_animation(UnitAnimationState::JUMPING);
	Tween::move(&pos.x, newX, JUMP_DURATION);
	Tween::move(&pos.y, stage->get_lane(currentLane).yPos, JUMP_DURATION)
		.setEase(Easing::easeOutBack);
}
bool Unit::try_leap() {
	int start = player_team() ? 0 : stage->laneCount - 1;
	int inc = player_team() ? 1 : -1;

	for (auto& gap : stage->get_lane(currentLane).gaps) {
		float leapRange = stats->get_augment(LEAP).value;
		float edge = player_team() ? gap.second : gap.first;
		float dist = (edge - pos.x) * stats->team;

		if (dist <= 0 || dist > leapRange || over_this_gap(gap, pos.x)) continue;

		float landingSpot = edge + LEDGE_SNAP * stats->team;
		start_special_animation(UnitAnimationState::JUMPING);
		Tween::move(&pos.x, landingSpot, LEAP_DURATION);
		return true;
	}

	return false;
}
void Unit::try_knockback(int oldHp, const UnitStats* enemyStats) {
	if (in_knockback()) return;
	if (met_knockback_threshold(oldHp, hp)) {
		shieldHp = (int)stats->get_augment(SHIELD).value;

		if (enemyStats->has_augment(AugmentType::SQUASH))
			push_squash_request();
		else if (enemyStats->has_augment(AugmentType::LAUNCH))
			push_launch_request();
		else
			knockback(false);
	}
	else if (trigger_augment(enemyStats, AugmentType::SHOVE))
		knockback(true);
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
	if (currentLane == stage->laneCount - 1) {
		knockback(false);
		return;
	}

	int lane = stage->find_lane_to_knock_to(*this, 1);
//	std::cout << "Found Lane to LAUNCH to, its Lane #" << lane << std::endl;
	if (lane == currentLane) knockback(false);
	else
		stage->push_move_request(*this, lane, stage->get_lane(lane).yPos, RequestType::LAUNCH);
}
bool Unit::try_push_jump_request() {
	int targetLane = currentLane + 1;
	if (stage->out_of_lane(targetLane, pos.x)) return false;
	float jumpRange = stats->get_augment(JUMP).value;

	for (auto& gap : stage->get_lane(targetLane).gaps) {
		if (!over_this_gap(gap, pos.x)) continue; // only jump on overhead gaps your on
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
bool Unit::enemy_in_range(float xPos, float minRange, float maxRange) {
	float dist = (xPos - pos.x) * stats->team;
	return dist >= minRange && dist <= maxRange;
}
bool Unit::enemy_is_in_sight_range() {
	float sightMultiplier = blinded() ? 0.5f : 1;
	float sightDist = stats->sightRange * sightMultiplier;

	if (enemy_in_range(stage->get_enemy_base(stats->team).pos.x, 0, sightDist))
		return true;

	auto [minLane, maxLane] = get_lane_range();
	
	for (int i = minLane; i < maxLane; i++) {
		std::vector<Unit>& enemyUnits = stage->get_lane_targets(i, stats->team);
		for (auto it = enemyUnits.begin(); it != enemyUnits.end(); ++it) {
			if (is_valid_target(*it, 0, sightDist)) return true;
		}
	}

	return false;
}
std::pair<int, int> Unit::get_lane_range() const {
	int min = std::max(currentLane - stats->minLaneReach, 0);
	int max = std::min(currentLane + stats->maxLaneReach + 1, stage->laneCount);
	return { min, max };
}
bool Unit::is_valid_target(const Unit& enemy, float minRange, float maxRange) {
	return !enemy.invincible() && !enemy.pending_death() &&
		enemy_in_range(enemy.pos.x, minRange, maxRange);
}
bool Unit::can_teleport() const { return !ancient_type() && stage->can_teleport(pos, currentLane, stats->team); }
bool Unit::over_gap() const { return stage->over_gap(currentLane, pos.x); }
bool Unit::try_proc_augment(const std::vector<Augment>& augments, AugmentType targetAugment) {
	for (auto& augment : augments) {
		if (augment.augType != targetAugment) continue;
		return roll_for_status() <= augment.percentage;
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

// Combat
void Unit::counter_surge(AugmentType& surgeType) {
	const Augment& augment = stats->get_augment(COUNTER_SURGE);
	float distance = augment.value;
	Augment newSurge(surgeType, distance, 0.f, augment.surgeLevel);
	stage->create_surge(*this, newSurge);
}
void Unit::on_kill(Unit& enemyUnit) {
	if (trigger_augment(stats, PLUNDER)) enemyUnit.statuses |= PLUNDER;
	if (stats->has_augment(CODE_BREAKER)) enemyUnit.statuses |= CODE_BREAKER;
}
float Unit::calculate_damage_reduction(const std::vector<Augment>& augments) {
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
	if (aug.augType == AugmentType::NONE) {
		std::cout << "Augment type was [NONE]. Cannot add effect" << std::endl;
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
	for (size_t i = activeStatuses.size(); i--;) {
		auto& status = activeStatuses[i];
		status.update(deltaTime);

		if (status.is_expired()) {
			statuses &= ~status.effect;
			activeStatuses[i] = std::move(activeStatuses.back());
			activeStatuses.pop_back();
		}
	}
}
int Unit::apply_effects(const std::vector<Augment>& augments, int dmg) {
	for (const Augment& augment : augments) {
		if (augment.is_status_effect() && !immune(augment.augType)) {
			if (!has_shield_up() && roll_for_status() < augment.percentage) {
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
	int dmg = attackingUnit.stats->dmg;

	if (attackingUnit.weakened()) dmg *= .5f;

	if (targeted_by_unit(attackingUnit))
		dmg = apply_effects(attackingUnit.stats->augments, dmg);

	if (attackingUnit.targeted_by_unit(*this))
		dmg = (int)(dmg * calculate_damage_reduction(stats->augments));
	
	return dmg;
}
bool Unit::damage_shield(int& dmg, const UnitStats* _stats) {
	if (trigger_augment(_stats, SHIELD_PIERCE)) {
		shieldHp = 0;
		return true; 
	}
	else if (dmg >= shieldHp) {
		dmg -= shieldHp;  
		shieldHp = 0;
		return true;
	} else {
		shieldHp -= dmg;  
		dmg = 0; 
		return false;
	}
}
bool Unit::take_damage(Unit& attackingUnit) {
	int dmg = calculate_damage_and_effects(attackingUnit);
	if (has_shield_up() && !damage_shield(dmg, attackingUnit.stats)) return false; // return if shield did not break

	if (targeted_by_unit(attackingUnit) && trigger_augment(attackingUnit.stats, VOID))
		dmg += stats->maxHp * attackingUnit.stats->get_augment(VOID).value;

	int oldHp = hp;
	hp -= dmg;
	std::cout << "ID: " << id << " - hit, oldHp: " << oldHp << " - newHp: " << hp << " - dmg taken: " << dmg << std::endl;

	try_knockback(oldHp, attackingUnit.stats);
	return hp <= 0 && !try_proc_survive();
}
bool Unit::take_damage(Surge& surge) {
	if (has_augment(COUNTER_SURGE) && surge.never_hit_unit(id))
		counter_surge(surge.surgeType);

	int dmg = surge.stats->dmg;

	if (targeted_by_unit(surge.stats->targetTypes))
		dmg = apply_effects(surge.stats->augments, dmg);
	if (surge.targeted_by_unit(stats->targetTypes))
		dmg = (int)(dmg * calculate_damage_reduction(stats->augments));

	if (has_shield_up() && !damage_shield(dmg, surge.stats)) return false; // return if shield did not BROKEN
	if (targeted_by_unit(surge.stats->targetTypes) && trigger_augment(surge.stats, VOID))
		dmg += stats->maxHp * surge.stats->get_augment(VOID).value;;

	int oldHp = hp;
	hp -= dmg;
	std::cout << "ID: " << id << " - hit, by a SURGE, oldHp: " << oldHp << " - dmg taken: " << dmg << " - newHp: " << hp << std::endl;

	try_knockback(oldHp, surge.stats);

	return hp <= 0 && !try_proc_survive();
}
void Unit::attack() {
	auto [minLane, maxLane] = get_lane_range();
	bool hitEnemy = false;

	for (int i = minLane; i < maxLane; i++) {
		std::vector<Unit>& enemyUnits = stage->get_lane_targets(i, stats->team);
		float minDist = abs(pos.x - stage->get_enemy_base(stats->team).pos.x);
		Unit* singleTargetedUnit = nullptr;

		for (auto it = enemyUnits.begin(); it != enemyUnits.end(); ++it) {
			if (is_valid_target(*it, stats->minAttackRange, stats->maxAttackRange)) {
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
}
void Unit::try_create_surge(bool hitEnemy) {
	if (!hitEnemy || !stats->has_surge()) return;
		
	for (auto& augment : stats->augments)
		if (augment.is_surge() && roll_for_status() <= augment.percentage)
			stage->create_surge(*this, augment);
}
void Unit::try_attack_enemy_base(bool& hitEnemy) {
	if (!stats->singleTarget || (stats->singleTarget && !hitEnemy)) {
		Base& enemyBase = stage->get_enemy_base(stats->team);
		if (enemy_in_range(enemyBase.pos.x, stats->minAttackRange, stats->maxAttackRange)) {
			enemyBase.take_damage(stats->dmg);
			hitEnemy = true;
		}
	}
}

// Tick
void Unit::tick(sf::RenderWindow& window, float deltaTime) {
	if (overloaded()) deltaTime *= 0.5f;

	attackCooldownTimer -= deltaTime;

	switch (animationState) {
	case UnitAnimationState::MOVING:
		moving_state(window, deltaTime);
		break;
	case UnitAnimationState::ATTACKING: 
		attack_state(window, deltaTime);
		break;
	case UnitAnimationState::IDILING:
		idling_state(window, deltaTime);
		break;
	case UnitAnimationState::KNOCKEDBACK:
		knockback_state(window, deltaTime);
		break;
	case UnitAnimationState::FALLING:
		falling_state(window, deltaTime);
		break;
	case UnitAnimationState::JUMPING:
		jumping_state(window, deltaTime);
		break;
	case UnitAnimationState::PHASE:
		phase_state(window, deltaTime);
		break;
	case UnitAnimationState::IS_PHASING:
		if (!tweening()) 
			start_special_animation(UnitAnimationState::PHASE);
	}
	update_status_effects(deltaTime);
}
void Unit::moving_state(sf::RenderWindow& window, float deltaTime) {
	draw(window, deltaTime);
	move(deltaTime);

	if (can_teleport())
		teleport();
	else if (can_fall()) {
		std::cout << "fall" << std::endl;
		if (rusted_tyoe()) start_animation(UnitAnimationState::IDILING);
		else push_fall_request();
	}
	else if (enemy_is_in_sight_range()) {
		std::cout << "found enemy" << std::endl;
		if (can_phase()) start_special_animation(UnitAnimationState::PHASE);
		else start_idle_or_attack_animation();
	}
	else {
		// in case Unit has both JUMP and LEAP, return if they succeed in jumping
		if (has_augment(JUMP) && currentLane < stage->laneCount - 1)
			if (try_push_jump_request()) return; 
		if (has_augment(LEAP)) try_leap();
	}
}
void Unit::attack_state(sf::RenderWindow& window, float deltaTime) {
	std::vector<AnimationEvent> events = draw(window, deltaTime);
	if (Animation::check_for_event(AnimationEvent::FINAL_FRAME, events)) {
		attackCooldownTimer = stats->attackTime;
		if (enemy_is_in_sight_range()) 
			start_idle_or_attack_animation();	
		else if (!rusted_tyoe() || (rusted_tyoe() && !can_fall()))
			start_animation(UnitAnimationState::MOVING);
		else
			start_animation(UnitAnimationState::IDILING);
	}
	if (Animation::check_for_event(AnimationEvent::UNIT_ATTACK, events))
		attack();
}
void Unit::idling_state(sf::RenderWindow& window, float deltaTime) {
	draw(window, deltaTime);
	if (enemy_is_in_sight_range())
		start_idle_or_attack_animation();
	else if (!rusted_tyoe() || (rusted_tyoe() && !stage->over_gap(currentLane, pos.x)))
		start_animation(UnitAnimationState::MOVING);
}
void Unit::knockback_state(sf::RenderWindow& window, float deltaTime) {
	draw(window, deltaTime);
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
}
void Unit::falling_state(sf::RenderWindow& window, float deltaTime) {
	draw(window, deltaTime);
	if (!tweening()) {
		std::cout << "Unit #" << id << " is DONE FALLING" << std::endl;
		if (hp <= 0) {
			std::cout << "destory after the fall" << std::endl;
			destroy_unit();
		}
		else if (enemy_is_in_sight_range())
			start_idle_or_attack_animation();
		else start_animation(UnitAnimationState::MOVING);
	}
}
void Unit::jumping_state(sf::RenderWindow& window, float deltaTime) {
	draw_special(window, deltaTime);
	if (!tweening()) {
		if (hp <= 0) destroy_unit();
		else if (enemy_is_in_sight_range())
			start_idle_or_attack_animation();
		else start_animation(UnitAnimationState::MOVING);
	}
}
void Unit::phase_state(sf::RenderWindow& window, float deltaTime) {
	auto events = draw_special(window, deltaTime);
	if (Animation::check_for_event(AnimationEvent::FINAL_FRAME, events)) {
		if (can_phase()) { // if can still phase
			statuses &= ~PHASE;
			float end = pos.x + stats->get_augment(PHASE).value * stats->team;
			auto [minWall, maxWall] = stage->get_walls(currentLane);
			end = std::clamp(end, minWall, maxWall);
			float timer = calculate_phase_timer(std::abs(pos.x - end));

			Tween::move(&pos.x, end, timer);
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


//	std::cout << "ID: " << id << " - hit, by UNIT, oldHp: " << oldHp << " - dmg taken : " << dmg << " - newHp : " << hp << std::endl;