#pragma once
#include "Stage.h"
#include "Loadout.h"
#include "Camera.h" 
#include "Observer.h"
#include "StageUI.h"
#include <iostream>
using Key = sf::Keyboard::Key;

const int MAX_BAG_LEVEL = 5;
const int BAG_UPGRADE_PORTION = 4;
const int BASE_BAG_COST = 4;

struct StageManager
{
	Camera& cam;
	StageUI& ui;
	Loadout& loadout;
	Stage stage = {};
	StageRecord stageRecorder = {};

	std::vector<Challenge> challenges = {};
	int clearedChallenges = 0;

	float timeSinceStart = 0.f;
	int selectedLane = 0;

	int parts = 0; // currency
	int partsPerSecond = 5;
	int currentBagLevel = 1;
	int bagUpgradeCost = 20;
	int baseBagUpgradeCost = 20;
	int bagCap = 500;
	int baseBagCap = 500;
	float partsIncTimer = 0;

	StageManager(Camera& cam, StageUI& ui, Loadout& loadout) : cam(cam), ui(ui), 
		loadout(loadout), effectSprites(make_effSpriteArr()) {
		for (int i = 0; i < 8; i++) effectSprites[i].setOrigin({ 16.f,16.f });
	};

	void unload_stage();
	void create_stage(const nlohmann::json& stageJson);
	void create_challenges(const nlohmann::json& stageJson);
	void set_texts();

	void update_game_ticks(float deltaTime);
	void process_move_requests();
	void spawn_enemies(float deltaTime);
	void increment_parts_and_notify(float deltaTime);

	void update_unit(float deltaTime);
	void update_ptr(float deltaTime);
	void update_base(float deltaTime);
	void update_projectiles(float deltaTime);
	void update_hitbox_visualizers(float deltaTime);

	void draw(sf::RenderWindow& window);

	void handle_events(sf::Event event);
	bool read_lane_switch_inputs(Key key);
	bool read_spawn_inputs(Key key);
	void read_button_inputs(Key key);
	void upgrade_bag();
	void pause();

	void try_spawn_death_surge(Unit& unit);
	void create_drop_box(int lane, const UnitStats* stats, UnitAniMap* aniMap);
	void try_create_cloner(Unit& unit);
	void collect_parts(Unit& unit);
	void handle_enemy_unit_death(Unit& unit);
	void handle_player_unit_death(Unit& unit);

	inline bool try_spend_parts(int partsToSpend) {
		if (parts < partsToSpend) return false;

		parts = std::max(parts - partsToSpend, 0);
		stageRecorder.add_parts_spent(partsToSpend);
		return true;
	}
	inline void gain_parts(int partsToGain) {
		parts = std::min(parts + partsToGain, bagCap);
		stageRecorder.add_parts_earned(partsToGain);
	}
	inline void notify_challenges() {
		int clears = 0;
		for (auto& challenge : challenges) {
			challenge.cleared = challenge.notify(*this);
			if (challenge.cleared) clears++;
		}

		if (clears != clearedChallenges)
			update_challenges_text(clears);
	}
	inline void update_challenges_text(int clears) {
		
		clearedChallenges = clears;

		if (clearedChallenges == challenges.size())
			ui.clearedChallengesText.setFillColor(sf::Color::Green);
		else
			ui.clearedChallengesText.setFillColor(sf::Color::Yellow);

		ui.clearedChallengesText.setString(std::format("Challenges Cleared: {}/{}",
			clearedChallenges, challenges.size()));
			
	}
	inline bool try_buy_upgrade_bag() {
		return currentBagLevel < MAX_BAG_LEVEL && try_spend_parts(bagUpgradeCost);
	}
	inline bool paused() const { return ui.paused; }
	inline bool try_fire_cannon() { return stage.playerBase.try_fire_cannon(); }
	inline bool can_fire_cannon() const { return stage.playerBase.on_cooldown(); }

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
	std::array<sf::Sprite, 8> effectSprites;
	 
	static std::array<sf::Sprite, 8> make_effSpriteArr() {
		return { sf::Sprite(TextureManager::t_statusIcons[0]),
			sf::Sprite(TextureManager::t_statusIcons[1]),
			sf::Sprite(TextureManager::t_statusIcons[2]),
			sf::Sprite(TextureManager::t_statusIcons[3]),
			sf::Sprite(TextureManager::t_statusIcons[4]),
			sf::Sprite(TextureManager::t_statusIcons[5]),
			sf::Sprite(TextureManager::t_statusIcons[6]),
			sf::Sprite(TextureManager::t_statusIcons[7]) };
	}
};
