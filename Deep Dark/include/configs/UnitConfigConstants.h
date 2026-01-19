#pragma once
#include <vcruntime.h>

constexpr size_t NULL_UNIT_INDEX = -1;

namespace UnitConfig {
	inline constexpr int PLAYER_TEAM = 1;
	inline constexpr int ENEMY_TEAM = -1;

	inline constexpr int TOTAL_PLAYER_UNITS = 7;
	inline constexpr int TOTAL_ENEMY_UNITS = 6;
	inline constexpr int TOTAL_UNITS = TOTAL_PLAYER_UNITS + TOTAL_ENEMY_UNITS;

	inline constexpr int NULL_ID = -1;
	inline constexpr int ENEMY_ID_OFFSET = 100;

	inline constexpr int GEAR1 = 1;
	inline constexpr int GEAR2 = 2;
	inline constexpr int GEAR3 = 3;

	inline constexpr int MAX_CORES = 3;

	inline constexpr int BASE_ANIM_COUNT = 6; // Minimum number of animations every Unit will have (Move, Idle, Attack, Knockback, Fall, Death)
	inline constexpr int SPECIAL_ANIM_COUNT = 5; // # of Unit animations for specific Abilities(Augments). They are Jump, Phase(windup and windown), Transform, and Summon.
	inline constexpr int TOTAL_ANIM_COUNT = BASE_ANIM_COUNT + SPECIAL_ANIM_COUNT; // Total # of different Unit animations

	// Knockback Forces
	inline constexpr float BASE_KB_FORCE = 1.f;
	inline constexpr float BOSS_SHOCKWAVE_KB_FORCE = 2.f;	// Force of Knockback from a boss Shockwave
	inline constexpr float HEAVYWEIGHT_KB_FORCE = 0.7f; // Knockback force reduction from being a heavyweight Unit
	inline constexpr float LIGHTWEIGHT_KB_FORCE = 1.5f; // Knockback force increase from being a lightweight Unit
	inline constexpr float BULLY_KB_FORCE = 1.5f; // Knockback force increase from being a hit by a Unit with the "Bully" Augment
	inline constexpr float SHOVE_KB_FORCE = 0.5f; // Knockback force when pushed by the "Shove" Augment
	inline constexpr float MAGNETIZE_KB_FORCE = -0.3f; // Knockback force when pulled by the "Magnet" Augment.
	inline constexpr float MIN_KB_TIMER_MULTIPLIER = 0.5f; // Minimum timer reduction of a knockback
	inline constexpr float MAX_KB_TIMER_MULTIPLIER = 1.35f; // Maximum timer increase of a knockback

	const float KNOCKBACK_DISTANCE = 50.0f; // Base distance of a knockback (when force = 1.f)
	const float KNOCKBACK_DURATION = .85f; // Base duration of a knockback (when force = 1.f)
	const float FALL_DURATION = 1.0f; // How long it takes to fall through lanes
	const float SQUASH_DURATION = 1.0f;
	const float LAUNCH_DISTANCE = 75.f;
	const float LAUNCH_DURATION = .45f;
	const float DROP_FROM_LAUNCH_DURATION = .525f;

	const float LEDGE_SNAP = 25.0f;
	const float JUMP_DURATION = 1.f;
	const float LEAP_DURATION = 0.7f;
}