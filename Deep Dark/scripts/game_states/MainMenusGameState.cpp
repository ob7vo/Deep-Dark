#include "MainMenusGameState.h"
#include "PreparationState.h"
#include "json.hpp"
#include <fstream>

MainMenusState::MainMenusState(Camera& cam) :
startMenu(cam), GameState(cam){
	startMenu.startBtn().onClick = [this]() {start_game(); };
}
void MainMenusState::update(float deltaTime) {
	startMenu.check_mouse_hover();
}
void MainMenusState::render() {
	startMenu.draw();
}
void MainMenusState::handle_events(sf::Event event) {
	if (event.is<sf::Event::MouseButtonPressed>())
		startMenu.register_click();
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
	MenuType prev = MenuType::START;
	nextStateEnterData = std::make_unique<PrepEnterData>(cur, prev);
	readyToEndState = true;
}
