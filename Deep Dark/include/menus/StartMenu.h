#pragma once
#include "Menu.h"


struct StartMenu : public Menu<UI::StartMenu::BTN_COUNT> {
	sf::Text startText = sf::Text(baseFont);
	bool openedSettings = false;
	
	explicit StartMenu(Camera& cam);
	~StartMenu() final = default;

	void draw() final;
	bool on_mouse_press(bool isM1) final;
	void check_mouse_hover() final;
	void reset_positions() final;

	void quit_game();
	inline void open_settings() { openedSettings = !openedSettings; }

	Button& startBtn();
	Button& quitBtn();
	Button& settingsBtn();
};