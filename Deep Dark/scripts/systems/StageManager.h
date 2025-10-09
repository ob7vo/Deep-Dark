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
const int MAX_BAG_LEVEL = 5;
const int BAG_UPGRADE_PORTION = 4;
const int BASE_BAG_COST = 4;

struct SMConstructor {
	//sf::Font partsFont;
//	SMConstructor() {partsFont.}
}; 
const sf::Font baseFont("fonts/KOMIKAX_.ttf");

struct StageManager
{
	Loadout loadout;
	Stage stage;
	StageRecord stageRecorder;

	sf::Text partsCountText = sf::Text(baseFont);
	sf::Text bagUpgradeCostText = sf::Text(baseFont);
	
	float timeSinceStart = 0.f;
	int selectedLane = 0;

	int parts = 0; // currency
	int partsPerSecond = 25;
	int currentBagLevel = 1;
	int bagUpgradeCost = 4;
	int baseBagUpgradeCost = 4;
	int bagCap = 500;
	int baseBagCap = 0;
	float partsIncTimer = 0;

	StageManager(const nlohmann::json& stageJson, std::vector<std::string>& loudoutSlots);
	void update_game_ticks(sf::RenderWindow& window, float deltaTime);
	void process_move_requests();
	void spawn_enemies(float deltaTime);
	void increment_parts(float deltaTime);
	void update_unit_ticks(sf::RenderWindow& window, float deltaTime);
	void update_ptr_ticks(sf::RenderWindow& window, float deltaTime);
	void update_base_ticks(sf::RenderWindow& window, float deltaTime);

	void only_draw(sf::RenderWindow& window);
	void update_ui(float deltaTime);
	void draw_ui(sf::RenderWindow& window);

	void handle_events(sf::Event event);
	void read_lane_switch_inputs(Key key);
	void read_spawn_inputs(Key key);
	void read_base_fire_input(Key key);
	void read_pouch_upgrade_input(Key key);

	void try_spawn_death_surge(Unit& unit);
	void create_drop_box(int lane, const UnitStats* stats, std::array<Animation, 5>* aniMap);
	void try_create_cloner(Unit& unit);
	void collect_parts(Unit& unit);
	void handle_enemy_unit_death(Unit& unit);
	void handle_player_unit_death(Unit& unit);

	inline void update_and_draw_ui(sf::RenderWindow& window, float deltaTime) {
		update_ui(deltaTime); 
		draw_ui(window);
	}
	inline void printy() { std::cout << selectedLane << std::endl; }
	inline bool try_spend_parts(int partsToSpend) {
		if (parts < partsToSpend) return false;

		parts = std::max(parts - partsToSpend, 0);
		stageRecorder.partsSpent += partsToSpend;
		return true;
	}
	inline void gain_parts(int partsToGain) {
		parts = std::min(parts + partsToGain, bagCap);
		stageRecorder.partsEarned += partsToGain;
	}
};

