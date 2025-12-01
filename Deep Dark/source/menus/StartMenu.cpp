#include "pch.h"
#include "StartMenu.h"
#include "Camera.h"
#include "UILayout.h"

using namespace UI::StartMenu;

StartMenu::StartMenu(Camera& cam) : Menu(cam) {
	startText.setPosition(cam.norm_to_pixels(START_TEXT_POS));
	startText.setString("DEEP DARK");

	settingsBtn().onClick = [this](bool m1) { if (m1) open_settings(); };
	quitBtn().onClick = [this](bool m1) { if (m1) quit_game(); };

	startBtn().setup(START_BTN_POS, START_BTN_SIZE, cam, TextureManager::t_startGameBtn);
	settingsBtn().setup(SETTINGS_BTN_POS, SETTINGS_BTN_SIZE, cam, TextureManager::t_settingsBtn);
	quitBtn().setup(QUIT_BTN_POS, QUIT_BTN_SIZE, cam, TextureManager::t_quitBtn);
}
void StartMenu::reset_positions() {
	startText.setPosition(cam.norm_to_pixels(START_TEXT_POS));

	startBtn().sprite.setPosition(cam.norm_to_pixels(START_BTN_POS));
	settingsBtn().sprite.setPosition(cam.norm_to_pixels(SETTINGS_BTN_POS));
	quitBtn().sprite.setPosition(cam.norm_to_pixels(QUIT_BTN_POS));
}

void StartMenu::draw() {
	if (openedSettings) {
		cam.queue_ui_draw(&settingsBtn().sprite);
		return;
	}
	cam.queue_ui_draw(&startText);
	buttonManager.draw(cam);
}
bool StartMenu::on_mouse_press(bool isM1) {
	int start = openedSettings ? 2 : 0;
	return buttonManager.on_mouse_press(cam.getMouseScreenPos(), isM1, start);
}
void StartMenu::check_mouse_hover() {
	if (openedSettings) {
		settingsBtn().check_mouse_hover(cam.getMouseScreenPos());
		return;
	}
	buttonManager.check_mouse_hover(cam.getMouseScreenPos());
}

void StartMenu::quit_game() { cam.close_window(); }

#pragma region Button Indexes
Button& StartMenu::startBtn() { return buttonManager.buttons[0]; }
Button& StartMenu::quitBtn() { return buttonManager.buttons[1]; }
Button& StartMenu::settingsBtn() { return buttonManager.buttons[2]; }
#pragma endregion