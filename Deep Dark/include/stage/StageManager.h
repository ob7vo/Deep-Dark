#pragma once
#include "Stage.h"
#include "Loadout.h"
#include "Camera.h" 
#include "StageWallet.h"
#include "Observer.h"
#include "StageUI.h"
#include <iostream>

using Key = sf::Keyboard::Key;

struct StageManager
{
	Camera& cam;
	StageUI& ui;
	Loadout& loadout;
	Stage stage = {};
	StageRecord stageRecorder = {};

	std::vector<Challenge> challenges = {};
	int clearedChallenges = 0;

	float oneSecondTimer = 0;
	int selectedLane = 0;

	StageWallet wallet = {};

	StageManager(Camera& cam, StageUI& ui, Loadout& loadout) : cam(cam), ui(ui), 
		loadout(loadout) {
		for (int i = 0; i < 8; i++) effectSprites[i].setOrigin({ 16.f,16.f });
	};

	void unload();

	void create_stage(const nlohmann::json& stageSetJson, int stageSet = 0);
	void create_challenges(const nlohmann::json& stageSetJson);
	void set_texts();

	void update_game_ticks(float deltaTime);
	void process_move_requests();
	void spawn_enemies();
	void increment_parts_and_notify(float deltaTime);

	void update_unit(float deltaTime);
	void update_entities(float deltaTime);
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

	void try_spawn_death_surge(const Unit& unit);
	void create_drop_box(int lane, const UnitStats* stats, UnitAniMap* aniMap);
	void try_create_cloner(const Unit& unit);
	void collect_parts(const Unit& unit);
	void handle_enemy_unit_death(const Unit& unit);
	void handle_player_unit_death(const Unit& unit);

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
	std::array<sf::Sprite, 8> effectSprites = make_effSpriteArr();
	 
	static std::array<sf::Sprite, 8> make_effSpriteArr() {
		std::array<sf::Sprite, 8> effArr = {
			sf::Sprite(TextureManager::t_statusIcons), sf::Sprite(TextureManager::t_statusIcons),
			sf::Sprite(TextureManager::t_statusIcons), sf::Sprite(TextureManager::t_statusIcons),
			sf::Sprite(TextureManager::t_statusIcons), sf::Sprite(TextureManager::t_statusIcons),
			sf::Sprite(TextureManager::t_statusIcons), sf::Sprite(TextureManager::t_statusIcons),
		};

		for (int i = 0; i < 8; i++)
			effArr[i].setTextureRect(TextureManager::r_workshopStatsIcons[i]);

		return effArr;
	}
};
