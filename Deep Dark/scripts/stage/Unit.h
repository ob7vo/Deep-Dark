#pragma once
#include "Animation.h"
#include "StatusEffect.h"
#include "Tween.h"
#include "UnitData.h"
#include <fstream>

const int PLAYER_TEAM = 1;
const int ENEMY_TEAM = -1;

struct Stage;
struct Surge;

class Unit {
private:
	Stage* stage; // Current Stage
	float attackCooldown = 0;
	UnitAnimationState animationState = UnitAnimationState::MOVING;
	//* state = nullptr;

	float aniTime = 0.f;
	int currentFrame = 0;
	std::array<Animation, 5>* aniMap; // Owned by Enemy Spawners and Loadout slots
	sf::Sprite sprite;

public:
	const UnitStats* stats; // Owned by Enemy Spawners and Loadout slots
	int hp = 0, shieldHp = 0;
	int hitIndex = 0;
	int kbIndex = 0;
	int id = -1;
	int currentLane = 0;
	sf::Vector2f pos;
	std::vector<StatusEffect> activeStatuses;

	sf::RectangleShape marker;

	size_t statuses = 0;
	DeathCause causeOfDeath = DeathCause::NONE;
	SpawnCategory spawnCategory = SpawnCategory::NORMAL;

	Unit(Stage* stage, sf::Vector2f startPos, int startingLane, const UnitStats* data,
		std::array<Animation, 5>* p_aniMap, int id = -1);
	Unit(Stage* stage, Surge& surge);
	~Unit() {
	//	cancel_tweens();
		if (move_req_check())
			std::cout << "MOVED UNIT WITH ID: " << id << " - address currently: " << this << std::endl;
		else 
			std::cout << "UNIT DIED, ID: " << id << " - at address: " << this << std::endl;
	} 

	using StateFunc = void (Unit::*)(sf::RenderWindow&, float);
	static const std::unordered_map<UnitAnimationState, StateFunc> stateMap;
	bool move_req_check();
	void destroy_unit();
	bool tweening();
	bool done_tweening_and_animating(int events);

	bool enemy_in_range(float xPos, float maxRange, float minRange);
	bool enemy_is_in_sight_range();
	bool is_valid_target(const Unit& enemy, float minRange, float maxRange);
	bool met_knockback_threshold(int oldHp, int hp);
	bool can_teleport() const;
	bool over_gap() const;
	bool try_proc_augment(const std::vector<Augment>& augments, AugmentType target, int hits = 0);
	bool rust_type_and_near_gap();
	bool try_terminate_unit(Unit& enemyUnit, int dmg = 0);

	void attack();
	void knockback(float force = 1.f);
	void squash(float newY);
	void launch(float newY);
	void fall(float newY);
	void teleport();
	void move(float deltaTime);
	void jump(float newX);
	bool try_leap();
	void warp(const UnitStats* enemyStats);

	float calculate_damage_reduction(const std::vector<Augment>& augments);
	int calculate_damage_and_effects(Unit& attackingUnit);
	int apply_effects(const std::vector<Augment>& statuses, int hitIndex, int dmg = 0);
	void add_status_effect(const Augment& aug);
	void update_status_effects(float deltaTime);
	bool damage_shield(int& dmg, const UnitStats* _stats);
	bool take_damage(Unit& attackingUnit);
	bool take_damage(Surge& surge);
	bool take_damage(int dmg);
	void try_create_surge(bool hitEnemy);
	void try_attack_enemy_base(bool& hitEnemy);
	bool try_proc_survive();
	void on_kill(Unit& enemyUnit);
	void counter_surge(AugmentType& surge);

	void start_animation(UnitAnimationState newState);
	void start_special_animation(UnitAnimationState specialState);
	void start_idle_or_attack_animation();
	int update_animation(float deltaTime);
	int update_animation_special(float deltaTime);

	void tick(float deltaTime);
	void moving_state(float deltaTime);
	void attack_state(float deltaTime);
	void idling_state(float deltaTime);
	void knockback_state(float deltaTime);
	void falling_state(float deltaTime);
	void jumping_state(float deltaTime);
	void phase_state(float deltaTime);
	void is_phasing_state(float deltaTime);
	void waiting_state(float deltaTime);

	void try_knockback(int oldHp, int hitIndex, const UnitStats* enemyStats);
	void push_fall_request();
	void push_squash_request();
	void push_launch_request();
	bool try_push_jump_request();
	void finish_launch_tween();
	std::pair<int, int> get_lane_reach();
	std::pair<int, int> get_lane_sight_range() const;

	inline bool player_team() const { return stats->team == PLAYER_TEAM; }
	inline bool invincible() const { return static_cast<int>(animationState) > 2; }
	inline bool pending_death() const {return hp <= 0;}
	inline bool dead() const { return hp <= 0 && animationState == UnitAnimationState::DYING; }
	inline bool falling() const { return animationState == UnitAnimationState::FALLING; }
	inline bool in_knockback() const { return animationState == UnitAnimationState::KNOCKEDBACK; }
	inline bool can_fall() const { return !floating_type() && over_gap(); }

	inline bool rusted_tyoe() const { return stats->rusted_tyoe(); }
	inline bool ancient_type() const { return stats->ancient_type(); }
	inline bool floating_type() const { return stats->floating_type(); }
	inline bool nano_type() const { return stats->unitTypes & NANO; }

	inline bool slowed() const { return statuses & AugmentType::SLOW; }
	inline bool overloaded() const { return statuses & AugmentType::OVERLOAD; }
	inline bool weakened() const { return statuses & AugmentType::WEAKEN; }
	inline bool blinded() const { return statuses & AugmentType::BLIND; }
	inline bool corroded() const { return statuses & AugmentType::CORRODE; }
	inline bool short_circuited() const { return statuses & AugmentType::SHORT_CIRCUIT; }
	inline bool infected() const { return statuses & AugmentType::VIRUS; }
	inline bool immune(AugmentType aug) const { return stats->immunities & aug && !infected(); }
	inline bool can_proc_status(Augment augment, int hitIndex = 0)
	{	return augment.is_status_effect() && !immune(augment.augType) && 
		!has_shield_up() && augment.can_hit(hitIndex);
	}

	inline bool targeted_by_unit(int enemyTargetTypes) const { return stats->targeted_by_unit(enemyTargetTypes); }
	inline bool targeted_by_unit(Unit& unit) 
	{ return stats->targeted_by_unit(unit.stats->targetTypes) && !unit.short_circuited(); }
	inline bool trigger_augment(const UnitStats* _stats, AugmentType target, int hits = 0) {
		return _stats->has_augment(target) && try_proc_augment(_stats->augments, target, hits);
	}
	inline bool is_summoned_unit() { return spawnCategory == SpawnCategory::SUMMON; }

	inline int get_dmg() { return stats->get_hit_stats(hitIndex).dmg; }
	inline std::pair<float, float> get_attack_range() { return stats->get_hit_stats(hitIndex).attackRange; }
	inline bool has_augment(AugmentType aug) const { return stats->has_augment(aug); }
	inline bool has_shield_up() const { return shieldHp > 0; }
	inline std::array<Animation, 5>* get_ani_array() { return aniMap; }

	inline bool can_phase() const { return statuses & PHASE; }
	inline sf::Sprite& get_sprite() { return sprite; }
	inline UnitAnimationState get_state() { return animationState; }

	static sf::Texture& get_default_texture() {
		static sf::Texture defaultTex;
		static bool initialized = false;
		if (!initialized) {
			initialized = true;
			bool shutTHeFuckUp = defaultTex.loadFromFile("sprites/defaultTexture.png");
			if (shutTHeFuckUp) return defaultTex;
		}
		return defaultTex;
	}
};

/*
struct UnitState {
	~UnitState() = default;
	virtual void state_tick(Unit& u, sf::RenderWindow& window, float deltaTime) = 0;
};
struct MovingUnitState : public UnitState {
	static MovingUnitState& get_state() {
		static MovingUnitState ins;
		return ins;
	}
	void state_tick(Unit& u, sf::RenderWindow& window, float deltaTime) override;
};
struct AttackingUnitState : public UnitState {
	static AttackingUnitState& get_state() {
		static AttackingUnitState ins;
		return ins;
	}
	void state_tick(Unit& u, sf::RenderWindow& window, float deltaTime) override;
};
*/