#pragma once
#include "Menu.h"
#include <SFML/Graphics/Sprite.hpp>

struct StageManager;
struct StageUI;

struct StagePauseMenu : public Menu<UI::StageUI::PAUSE_MENU_BTN_COUNT> {
	StageUI& stageUI;
	sf::Text pauseText = sf::Text(baseFont);
	sf::Sprite pauseMenuSprite = sf::Sprite(defTex);

	StagePauseMenu(Camera& cam, StageUI& ui);
	~StagePauseMenu() final = default;

	void draw() final;
	void reset_positions() final;

	void close_menu();
	void close_game();

	inline Button& closeGameBtn() { return buttonManager.buttons[0]; }
	inline Button& closeMenuBtn() { return buttonManager.buttons[1]; }
};

struct StageUI : public Menu<UI::StageUI::UI_BTN_COUNT> {
	StageManager* stageManager;
	StagePauseMenu pauseMenu;

	sf::Text partsCountText = sf::Text(baseFont);
	sf::Text bagUpgradeCostText = sf::Text(baseFont);
	sf::Text clearedChallengesText = sf::Text(baseFont);

	explicit StageUI(Camera& cam);
	~StageUI() final = default;

	void create_buttons();

	void draw() final;
	bool on_mouse_press(bool isM1) final;
	void check_mouse_hover() final;
	void reset_positions() final;

	void pause();
	void upgrade_bag();
	void fire_cannon();

	inline Button& fireCannonBtn() { return buttonManager.buttons[2]; }
	inline Button& upgradeBagBtn() { return buttonManager.buttons[1]; }
	inline Button& pauseBtn() { return buttonManager.buttons[0]; }
};