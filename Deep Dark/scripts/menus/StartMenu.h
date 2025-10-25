#pragma once
#include "Menu.h"

const int START_MENU_BTNS = 3;

struct StartMenu : public Menu<START_MENU_BTNS> {
	sf::Text startText = sf::Text(baseFont);
	bool openedSettings = false;

	StartMenu(Camera& cam);
	~StartMenu() = default;

	void draw() override;
	void register_click() override;
	void check_mouse_hover() override;

	void start_game();
	void quit_game();
	void open_settings();

	inline Button& startBtn() { return buttonManager.buttons[0]; }
	inline Button& quitBtn() { return buttonManager.buttons[1]; }
	inline Button& settingsBtn() { return buttonManager.buttons[2]; }
};