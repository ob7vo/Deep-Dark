#pragma once
#include "Stage.h"
#include "Loadout.h"
#include <iostream>
using Key = sf::Keyboard::Key;

const std::array<Key, 10> numberKeys = {
	Key::Num1,
	Key::Num2,  // Index 1 -> Key 2
	Key::Num3,  // Index 2 -> Key 3
	Key::Num4,  // Index 3 -> Key 4
	Key::Num5,  // Index 4 -> Key 5
	Key::Num6,  // Index 5 -> Key 6
	Key::Num7,  // Index 6 -> Key 7
	Key::Num8,  // Index 7 -> Key 8
	Key::Num9,  // Index 8 -> Key 9
	Key::Num0   // Index 9 -> Key 0
};

struct StageManager
{
	Loadout loadout;
	Stage stage;
	float timeSinceStart = 0.f;
	int selectedLane = 0;
	int parts = 0; // currency 
	int walletCap = 500;

	StageManager(const nlohmann::json& stageJson, std::vector<std::string>& loudoutSlots);
	void update_game_ticks(sf::RenderWindow& window, float deltaTime);
	void process_move_requests();
	void spawn_enemies(float deltaTime);
	void update_unit_ticks(sf::RenderWindow& window, float deltaTime);
	void update_surge_ticks(sf::RenderWindow& window, float deltaTime);
	void update_base_ticks(sf::RenderWindow& window, float deltaTime);
	void only_draw(sf::RenderWindow& window);
	void handle_events(sf::Event event);
	void read_lane_switch_inputs(Key key);
	void read_spawn_inputs(Key key);
	void read_base_fire_input(Key key);

	void try_spawn_death_surge(Unit& unit);
	void try_revive_unit(Unit& unit);
	void collect_parts(Unit& unit);
	void handle_enemy_unit_death(Unit& unit);
	void handle_player_unit_death(Unit& unit);

	inline void printy() { std::cout << selectedLane << std::endl; }
};

