#include "StartMenu.h"

StartMenu::StartMenu(Camera& cam) : Menu(cam) {

}

void StartMenu::draw() {
	if (openedSettings) {
		cam.queue_ui_draw(&settingsBtn().sprite);
		return;
	}
	cam.queue_temp_ui_draw(&startText);
	buttonManager.draw(cam);
}
void StartMenu::register_click() {
	int start = openedSettings ? 2 : 0;
	buttonManager.register_click(cam, start);
}
void StartMenu::check_mouse_hover() {
	if (openedSettings) {
		settingsBtn().check_mouse_hover(cam.get_mouse_screen_position());
		return;
	}
	buttonManager.check_mouse_hover(cam);
}

void StartMenu::start_game() {

}