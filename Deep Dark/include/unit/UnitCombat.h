#pragma once
#include <vector>

class Unit;

struct UnitCombat {
	float cooldown = 0;
	int hitIndex = 0;

	UnitCombat() = default;


	void attack(Unit& attackerUnit);
	bool process_attack_on_lanes(Unit& attackerUnit) const;
	bool attack_lane(Unit& attacker, int laneIndex) const;
	bool attack_single_target(Unit& attackerUnit,std::vector<Unit>& enemies) const;
	bool attack_all_targets(Unit& attackerUnit, std::vector<Unit>& enemies) const;
	void handle_post_attack_effects(Unit& attacker, bool hitEnemy) const;

	void try_create_surge(Unit& attackerUnit, bool hitEnemy) const;
	void try_attack_enemy_base(Unit& attackerUnit, bool& hitEnemy) const;
	void try_create_projectile(Unit& attackerUnit) const;
	bool try_terminate_unit(Unit& attackerUnit, Unit& enemyUnit, int dmg = 0) const;
	void on_kill(Unit& attackerUnit, Unit& enemyUnit) const;
};