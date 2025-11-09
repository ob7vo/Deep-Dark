#include "StartMenu.h"
#include "UILayout.h"

using namespace UI::StartMenu;

StartMenu::StartMenu(Camera& cam) : Menu(cam) {
	startText.setPosition(cam.norm_to_pixels(START_TEXT_POS));
	startText.setString("DEEP DARK");

	settingsBtn().onClick = [this](bool m1) { if (m1) open_settings(); };
	quitBtn().onClick = [this](bool m1) { if (m1) quit_game(); };

	std::string texPath1 = "sprites/ui/start_menu/start_btn.png";
	startBtn().set_ui_params(START_BTN_POS, START_BTN_SIZE, texPath1, cam);
	std::string texPath2 = "sprites/ui/start_menu/settings_btn.png";
	settingsBtn().set_ui_params(SETTINGS_BTN_POS, SETTINGS_BTN_SIZE, texPath2, cam);
	std::string texPath3 = "sprites/ui/start_menu/quit_btn.png";
	quitBtn().set_ui_params(QUIT_BTN_POS, QUIT_BTN_SIZE, texPath3, cam);
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
