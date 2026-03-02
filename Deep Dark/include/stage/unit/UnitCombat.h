#pragma once
#include <vector>

struct Stage;
class Unit;
struct Augment;

struct UnitCombat {
	Stage* stage = nullptr;
	Unit& attacker;

	int kills = 0;
	float cooldown = 0;
	// Current hit # that the unit is on
	// Used for multi hits to work and to activate abilities.
	int hitIndex = 0;

	explicit UnitCombat(Unit& ownerUnit) : attacker(ownerUnit) {}
	~UnitCombat() = default;

	inline void setup() {
		kills = hitIndex = 0;
		cooldown = 0.f;
	};

	void attack();
	bool process_attack_on_lanes();
	bool attack_lane(int laneIndex);
	bool attack_single_target(const std::vector<size_t>& enemies) ;
	bool attack_all_targets(const std::vector<size_t>& enemies);
	void handle_post_attack_effects(bool hitEnemy) const;

	void try_create_surge(bool hitEnemy) const;
	void try_attack_enemy_base(bool& hitEnemy) const;
	void try_create_projectile() const ;
	bool try_terminate_unit(const Unit& hitUnit, int dmg = 0) const;
	void on_kill(Unit& enemyUnit);

	void self_destruct(const Augment& selfDestruct) ;
	void discharge(int oldHp);
};