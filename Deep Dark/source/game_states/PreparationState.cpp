#include "PreparationState.h"
#include "StageGameState.h"
#include <fstream>
#include <json.hpp>

PreparationState::PreparationState(Camera& cam) : GameState(cam),
stageSelect(cam), armoryMenu(cam), workshopMenu(cam){
	for (int i = 0; i < STAGES; i++)
		stageSelect.stageBtn(i).onClick = [this, i](bool m1) { if (m1) start_stage(i); };
	stageSelect.armoryBtn().onClick = [this](bool m1) { if (m1) switch_menu(MenuType::ARMORY_EQUIP); };
	armoryMenu.returnBtn().onClick = [this](bool m1) { if (m1) switch_menu(prevMenuType); };
	workshopMenu.return_btn().onClick = [this](bool m1) { if (m1) switch_menu(prevMenuType); };

	for (int i = 0; i < 3; i++) {
		armoryMenu.unitSelectionBtn(i).onClick = [i, this](bool isM1) {
			if (isM1) armoryMenu.drag_unit(i);
			else {
				workshopMenu.setup_workshop_unit(i, armoryMenu.unitSelectionForms[i]);
				switch_menu(MenuType::WORKSHOP_MENU);
			}
			};
	}
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

MenuBase* PreparationState::get_menu() {
	cam.reset();
	std::cout << "getting menu: " << (int)curMenuType << std::endl;

	switch (curMenuType) {
	case MenuType::STAGE_SELECT:
		cam.change_lock(false);
		return &stageSelect;
	case MenuType::ARMORY_EQUIP:
		cam.change_lock(true);
		return &armoryMenu;
	case MenuType::WORKSHOP_MENU:
		cam.change_lock(true);
		return &workshopMenu;
	case MenuType::MAIN_MENU:
		readyToEndState = true;
		nextStateEnterData = std::make_unique<OnStateEnterData>(GameState::Type::MAIN_MENU);
		return menu;
	}

	std::cout << "Could not get Menu: " << (int)curMenuType << std::endl;
	return nullptr;
}
void PreparationState::start_stage(int stage) {
	std::string jsonPath = std::format("configs/stage_data/stage_{}.json", stage+1);
	std::ifstream stageFile(jsonPath);

	if (!stageFile.is_open()) {
		std::cerr << "Error: Could not open file: " << jsonPath << std::endl;
		return;
	}
	else if (armoryMenu.filledUnitSlots == 0) {
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