#pragma once
#include "UnitMovement.h"
#include "UnitStatus.h"
#include "UnitAnimation.h"
#include "UnitCombat.h"
#include "UnitData.h"

const int PLAYER_TEAM = 1;
const int ENEMY_TEAM = -1;

const int MAX_EFFECTS = 4;

struct Stage;
struct Lane;
struct Surge;

class Unit {
public:
	Stage* stage; // Current Stage
	const UnitStats* stats; // Owned by Enemy Spawners and Loadout slots
	UnitStatus status;
	UnitAnimation anim;
	UnitMovement movement;
	UnitCombat combat;

	int id = -1;
	
	DeathCause causeOfDeath = DeathCause::NONE;
	UnitSpawnType spawnCategory = UnitSpawnType::NORMAL;

	Unit(Stage* stage, sf::Vector2f startPos, int startingLane, const UnitStats* data,
		UnitAniMap* p_aniMap, int id = -1);
	~Unit() = default;

	Unit(Unit&&) = default;
	Unit& operator=(Unit&&) = default;

	Unit(const Unit&) = delete;
	Unit& operator=(const Unit&) = delete;

	bool move_req_check();
	void destroy_unit(DeathCause deathCause = DeathCause::NONE);
	void call_death_anim(DeathCause deathCause = DeathCause::NONE);

	bool enemy_in_range(float xPos, float maxRange, float minRange) const;
	bool enemy_is_in_sight_range() const;
	bool found_valid_target(const Unit& enemy, float minRange, float maxRange) const;
	bool over_gap() const;
	bool rust_type_and_near_gap() const;
	bool can_make_surge(const Augment& aug) const;

	void tick(float deltaTime);
	void moving_state(float deltaTime);
	void attack_state(float deltaTime);
	void idling_state(float deltaTime);
	void knockback_state(float deltaTime);
	void falling_state(float deltaTime);
	void jumping_state(float deltaTime);
	void transform_state(float deltaTime);
	void phase_windup_state(float dt);
	void phase_active_state(float deltaTime);
	void phase_winddown_state(float deltaTime);
	void death_state(float deltaTime);

	void try_knockback(int oldHp, int enemyHitIndex, const UnitStats* enemyStats);

	std::pair<int, int> get_lane_reach() const;
	std::pair<int, int> get_lane_sight_range() const;
	std::pair<float, float> getHurtboxEdges() const;

	inline bool player_team() const { return stats->team == PLAYER_TEAM; }
	inline bool dead() const { return status.hp <= 0 && anim.dead(); }
	inline bool can_fall() const { return !stats->floating_type() && over_gap(); }

	inline bool immune(AugmentType aug) const { return has(stats->immunities & aug) && !status.infected(); }

	inline bool is_targeted(UnitType targetTypes) const { return has(stats->unitTypes, targetTypes); }
	inline bool targeted_by_unit(const Unit& attacker) const
	{ return stats->is_targeted(attacker.stats->targetTypes) && !attacker.status.short_circuited(); }

	inline int get_dmg() const { return stats->get_hit_stats(combat.hitIndex).dmg; }
	inline const sf::Vector2f& get_pos() const { return movement.pos; }
	inline int get_lane() const { return movement.currentLane; }
	inline sf::FloatRect getBounds() const { return anim.get_sprite().getGlobalBounds(); }

	inline std::pair<float, float> get_attack_range() const 
	{ return stats->get_hit_stats(combat.hitIndex).attackRange; }
	inline float get_dir() const { return stats->get_dir(); }
};
