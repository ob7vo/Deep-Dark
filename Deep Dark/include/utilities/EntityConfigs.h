#pragma once
#include <SFML/System/Vector2.hpp>

namespace EntityConfigs {
	namespace Surges {
		inline constexpr float SHOCK_WAVE_WIDTH = 25.f; // The Width of a Shock Wave's hitbxx
		inline constexpr float SHOCK_WAVE_Lv1_DISTANCE = 75.f; // How far a Shock Wave travels at Level 1
		inline constexpr float SHOCK_WAVE_DISTANCE_PER_LEVEL = 50.f; // How farther a Shock Wave travels per Level increase
		inline constexpr float SHOCK_WAVE_Lv1_TIMER = .75f;// How long it takes a Shock Wave to fully travel its distance at Lv 1
		inline constexpr float SHOCK_WAVE_TIME_PER_LEVEL = .50f; // How much longer it takes a Shock Wave to fully travel its distance per Level Increase

		inline constexpr float FIREWALL_WIDTH = 30.f; // The Width of a FireWall's hitbox
		inline constexpr float FIREWALL_TIMER_PER_LEVEL = .66f; // How long a FireWall lasts (per level).

		inline constexpr float ORBITAL_STRIKE_WIDTH = 40.f; // The Width of a Orbital Strike's hitbox
	}
	namespace Traps {
		inline constexpr sf::Vector2f TELEPORTER_HITBOX = { 32.f, 32.f };
	}
	namespace Bases {
		inline constexpr float MAX_FIRE_WALL_DIST = 10000.f; // HMaximum distance a Fire Wall Cannon can hit a Unit
		inline constexpr float FIRE_WALL_SPAWN_PADDING = 7.5f; // Padding to ensure a Cannon's Fire Wall won't spawn directly on the hit Unit

		inline constexpr float ORBITAL_CANNON_SPACING = 45.f; // Spacing between a OS Cannon's Orbital Strikes
		inline constexpr int ORBITAL_STRIKES = 6; // # of Orbital Strikes spawned by a Cannon
		inline constexpr int BOTTOM_LANE = 0; // Lane that the Orbital Strikes from a cannon spawn on

		inline constexpr float SHORT_CIRCUIT_CANNON_RANGE = 300.f; // Range of a Short Circuit cannon
	}
}