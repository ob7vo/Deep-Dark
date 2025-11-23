#include "MainMenusGameState.h"
#include "PreparationState.h"
#include <json.hpp>
#include <fstream>

MainMenusState::MainMenusState(Camera& cam) :
startMenu(cam), GameState(cam){
	startMenu.reset_positions();
	startMenu.startBtn().onClick = [this](bool m1) { if (m1)start_game(); };
}
void MainMenusState::update(float deltaTime) {
	startMenu.check_mouse_hover();
}
void MainMenusState::render() {
	startMenu.draw();
}
void MainMenusState::handle_events(sf::Event event) {
	if (auto click = event.getIf<sf::Event::MouseButtonPressed>())
		clicked = startMenu.on_mouse_press(is_M1(click));
}
void MainMenusState::on_enter(OnStateEnterData* enterData) {
	startMenu.openedSettings = false;
	cam.change_lock(true);
}
void MainMenusState::on_exit() {
	startMenu.reset_positions();
}

void MainMenusState::start_game() {
	MenuType cur = MenuType::STAGE_SELECT;
	MenuType prev = MenuType::MAIN_MENU;
	nextStateEnterData = std::make_unique<PrepEnterData>(cur, prev);
	readyToEndState = true;
}
