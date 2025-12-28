#pragma once
#include <vector>

struct Stage;
class Unit;
struct Augment;

struct UnitCombat {
	Stage* stage = nullptr;
	float cooldown = 0;
	// Current hit # that the unit is on
	// Used for multi hits to work and to activate abilities.
	int hitIndex = 0;

	UnitCombat() = default;
	~UnitCombat() = default;

	inline void setup() { cooldown = hitIndex = 0; }

	void attack(Unit& attackerUnit);
	bool process_attack_on_lanes(Unit& attackerUnit) const;
	bool attack_lane(Unit& attacker, int laneIndex) const;
	bool attack_single_target(Unit& attackerUnit,const std::vector<size_t>& enemies) const;
	bool attack_all_targets(Unit& attackerUnit, const std::vector<size_t>& enemies) const;
	void handle_post_attack_effects(Unit& attacker, bool hitEnemy) const;

	void try_create_surge(Unit& attackerUnit, bool hitEnemy) const;
	void try_attack_enemy_base(Unit& attackerUnit, bool& hitEnemy) const;
	void try_create_projectile(Unit& attackerUnit) const;
	bool try_terminate_unit(const Unit& attackerUnit, const Unit& hitUnit, int dmg = 0) const;
	void on_kill(Unit& attackerUnit, Unit& enemyUnit) const;

	void self_destruct(Unit& unit, const Augment& selfDestruct) const;
};