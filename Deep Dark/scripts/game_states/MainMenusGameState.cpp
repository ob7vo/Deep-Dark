#include "MainMenusGameState.h"
#include "StageGameState.h"
#include "json.hpp"
#include <fstream>

MainMenusGameState::MainMenusGameState(Camera& cam) :
startMenu(cam), GameState(cam){
	startMenu.startBtn().onClick = [this]() {start_game(); };
}
void MainMenusGameState::update(float deltaTime) {
	startMenu.check_mouse_hover();
}
void MainMenusGameState::render() {
	startMenu.draw();
}
void MainMenusGameState::handle_events(sf::Event event) {
	if (event.is<sf::Event::MouseButtonPressed>())
		startMenu.register_click();
}
void MainMenusGameState::on_enter(OnStateEnterData* enterData) {
	startMenu.openedSettings = false;
}
void MainMenusGameState::on_exit() {
	startMenu.reset_positions();
}

void MainMenusGameState::start_game() {
	std::vector<std::string> slots = { "configs/player_units/soldier/soldier.json" };
	std::ifstream stageFile("configs/stage_data/stage_1.json");
	nlohmann::json stageJson = nlohmann::json::parse(stageFile);
	stageFile.close();

	nextStateEnterData = std::make_unique<StageEnterData>(stageJson, slots);
	readyToEndState = true;
}
