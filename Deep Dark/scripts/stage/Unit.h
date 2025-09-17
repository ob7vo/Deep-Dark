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
	float attackCooldownTimer;
	UnitAnimationState animationState;

	float aniTime;
	int currentFrame;
	std::array<Animation, 5>* aniMap; // Owned by Enemy Spawners and Loadout slots
	sf::Sprite sprite;

public:
	const UnitStats* stats; // Owned by Enemy Spawners and Loadout slots
	int hp, shieldHp;
	int kbIndex;
	int id = -1;
	int currentLane;
	sf::Vector2f pos;
	std::vector<StatusEffect> activeStatuses;

	sf::RectangleShape marker;

	int statuses;

	Unit(Stage* stage, sf::Vector2f startPos, int startingLane, const UnitStats* data,
		std::array<Animation, 5>* p_aniMap, int id = -1);
	~Unit() {
	//	cancel_tweens();
		std::cout << "dead, id: " << id << std::endl;
		/*
		if (move_req_check())
			std::cout << "MOVED UNIT WITH ID: " << id << " - address currently: " << this << std::endl;
		else 
			std::cout << "UNIT DIED, ID: " << id << " - at address: " << this << std::endl;
			*/
	} 
	bool move_req_check();
	void destroy_unit();

	bool enemy_in_range(float xPos, float maxRange, float minRange);
	bool enemy_is_in_sight_range();
	bool is_valid_target(const Unit& enemy, float minRange, float maxRange);
	bool met_knockback_threshold(int oldHp, int hp);
	bool can_teleport() const;
	bool over_gap() const;
	bool try_proc_augment(const std::vector<Augment>& augments, AugmentType targetAugment);

	void attack();
	void knockback(bool shove);
	void squash(float newY);
	void launch(float newY);
	void fall(float newY);
	void teleport();
	void move(float deltaTime);
	void jump(float newX);
	bool try_leap();

	float calculate_damage_reduction(const std::vector<Augment>& augments);
	int calculate_damage_and_effects(Unit& attackingUnit);
	int apply_effects(const std::vector<Augment>& statuses, int dmg = 0);
	void add_status_effect(const Augment& aug);
	void update_status_effects(float deltaTime);
	bool damage_shield(int& dmg, const UnitStats* _stats);
	bool take_damage(Unit& attackingUnit);
	bool take_damage(Surge& surge);
	void try_create_surge(bool hitEnemy);
	void try_attack_enemy_base(bool& hitEnemy);
	bool try_proc_survive();
	void on_kill(Unit& enemyUnit);
	void counter_surge(AugmentType& surge);

	void start_animation(UnitAnimationState newState);
	void start_special_animation(UnitAnimationState specialState);
	void start_idle_or_attack_animation();
	std::vector<AnimationEvent> draw(sf::RenderWindow& window, float deltaTime);
	std::vector<AnimationEvent> draw_special(sf::RenderWindow& window, float deltaTime);

	void tick(sf::RenderWindow& window, float deltaTime);
	void moving_state(sf::RenderWindow& window, float deltaTime);
	void attack_state(sf::RenderWindow& window, float deltaTime);
	void idling_state(sf::RenderWindow& window, float deltaTime);
	void knockback_state(sf::RenderWindow& window, float deltaTime);
	void falling_state(sf::RenderWindow& window, float deltaTime);
	void jumping_state(sf::RenderWindow& window, float deltaTime);
	void phase_state(sf::RenderWindow& window, float deltaTime);

	void try_knockback(int oldHp, const UnitStats* enemyStats);
	void push_fall_request();
	void push_squash_request();
	void push_launch_request();
	bool try_push_jump_request();
	std::pair<int, int> get_lane_range() const;

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
	inline bool slowed() const { return statuses & AugmentType::SLOW; }
	inline bool overloaded() const { return statuses & AugmentType::OVERLOAD; }
	inline bool weakened() const { return statuses & AugmentType::WEAKEN; }
	inline bool blinded() const { return statuses & AugmentType::BLIND; }
	inline bool short_circuited() const { return statuses & AugmentType::SHORT_CIRCUIT; }
	inline bool infected() const { return statuses & AugmentType::VIRUS; }
	inline bool immune(AugmentType aug) const { return stats->immunities & aug && !infected(); }
	inline bool targeted_by_unit(int enemyTargetTypes) const { return stats->targeted_by_unit(enemyTargetTypes); }
	inline bool targeted_by_unit(Unit& unit) { return stats->targeted_by_unit(unit.stats->targetTypes) && !unit.short_circuited(); }
	inline bool tweening() { return Tween::isTweening(&pos) || Tween::isTweening(&pos.y) || Tween::isTweening(&pos.x);	}
	inline void cancel_tweens() { Tween::cancel(&pos); Tween::cancel(&pos.x); Tween::cancel(&pos.y); }
	inline bool trigger_augment(const UnitStats* _stats, AugmentType target) {
		return _stats->has_augment(target) && try_proc_augment(_stats->augments, target);
	}
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