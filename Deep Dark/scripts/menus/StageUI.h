#pragma once
#include "Menu.h"

const int STAGE_UI_BUTTONS = 3;
const int STAGE_PAUSE_MENU_BUTTONS = 2;
struct StageManager;
struct StageUI;

struct StagePauseMenu : public Menu<STAGE_PAUSE_MENU_BUTTONS> {
	StageUI& stageUI;
	sf::Text pauseText = sf::Text(baseFont);

	sf::Texture pauseMenuTexture = defTexture;
	sf::Sprite pauseMenuSprite = sf::Sprite(defTexture);

	StagePauseMenu(Camera& cam, StageUI& ui);

	void draw() override;

	void close_menu();
	void close_game();

	inline Button& closeGameBtn() { return buttonManager.buttons[0]; }
	inline Button& closeMenuBtn() { return buttonManager.buttons[1]; }
};

struct StageUI : public Menu<STAGE_UI_BUTTONS> {
	StageManager* stageManager;
	StagePauseMenu pauseMenu;

	sf::Text partsCountText = sf::Text(baseFont);
	sf::Text bagUpgradeCostText = sf::Text(baseFont);
	sf::Text clearedChallengesText = sf::Text(baseFont);

	StageUI(Camera& cam);
	void create_buttons();

	void draw() override;
	void register_click() override;
	void check_mouse_hover() override;

	void pause();
	void upgrade_bag();
	void fire_cannon();

	inline Button& fireCannonBtn() { return buttonManager.buttons[2]; }
	inline Button& upgradeBagBtn() { return buttonManager.buttons[1]; }
	inline Button& pauseBtn() { return buttonManager.buttons[0]; }
};