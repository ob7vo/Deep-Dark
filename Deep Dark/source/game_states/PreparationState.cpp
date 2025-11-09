#include "PreparationState.h"
#include "StageGameState.h"
#include <fstream>
#include "json.hpp"

PreparationState::PreparationState(Camera& cam) : GameState(cam),
stageSelect(cam), armoryMenu(cam){
	for (int i = 0; i < STAGES; i++)
		stageSelect.stageBtn(i).onClick = [this, i](bool m1) { if (m1) start_stage(i); };
	armoryMenu.equippedUnits = { {0,2}, {1,1} };
}

void PreparationState::update(float deltaTime) {
	menu->check_mouse_hover();
	menu->update(deltaTime);
}
void PreparationState::render() {
	menu->draw();
}
void PreparationState::handle_events(sf::Event event) {
	if (auto click = event.getIf<sf::Event::MouseButtonPressed>())
		clicked = menu->on_mouse_press(is_M1(click));
	if (auto release = event.getIf<sf::Event::MouseButtonReleased>())
		menu->on_mouse_release(is_M1(release));
}

void PreparationState::start_stage(int stage) {
	std::string jsonPath = std::format("configs/stage_data/stage_{}.json", stage+1);
	std::ifstream stageFile(jsonPath);

	if (!stageFile.is_open()) {
		std::cerr << "Error: Could not open file: " << jsonPath << std::endl;
		return;
	}
	else if (armoryMenu.equippedUnits.size() == 0) {
		std::cerr << "Error: loadout is empty. \n";
		return;
	}

	nlohmann::json stageJson = nlohmann::json::parse(stageFile);
	stageFile.close();

	nextStateEnterData = std::make_unique<StageEnterData>(stageJson, armoryMenu);
	readyToEndState = true;
	std::cout << "starting stage #" << stage + 1 << std::endl;
}
void PreparationState::on_enter(OnStateEnterData* enterData) {
	if (PrepEnterData* prepData = dynamic_cast<PrepEnterData*>(enterData)) {
		curMenuType = prepData->openningMenuType;
		prevMenuType = prepData->prevMenuType;
		menu = get_menu();
	}
	else {
		std::cout << "Enter Data is not Prep Data" << std::endl;
		return;
	}
}
void PreparationState::on_exit() {
}