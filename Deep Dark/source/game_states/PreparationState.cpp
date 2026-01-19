#include "pch.h"
#include "PreparationState.h"
#include "StageGameState.h"

PreparationState::PreparationState(Camera& cam) : GameState(cam),
stageSelect(cam), armoryMenu(cam), workshopMenu(cam)
{
	stageSelect.reset_positions();
	armoryMenu.reset_positions();
	workshopMenu.reset_positions();

	setup_button_functions();
}
void PreparationState::setup_button_functions() {
#pragma region StageSelect
	for (int i = 0; i < STAGES; i++)
		stageSelect.stageNodeBtn(i).onClick = [this, i](bool m1) { if (m1) start_stage_set(i, 0); };
	stageSelect.enterArmoryBtn().onClick = [this](bool m1) {
		if (m1) {
			armoryMenu.inStageMode = true;
			armoryMenu.stageSetMenu.setup_menu(stageSelect.selectedStage);
			switch_menu(MenuType::ARMORY_EQUIP);
		}
		};
#pragma endregion

#pragma region ArmoryMenu
	armoryMenu.returnBtn().onClick = [this](bool m1) 
		{ if (m1) switch_menu(prevMenuType); };

	armoryMenu.stageSetMenu.startStageBtn().onClick = [this](bool m1) {
		if (m1) start_stage_set(armoryMenu.stageSetMenu.stageId, armoryMenu.stageSetMenu.stageSet);
		};
	armoryMenu.stageSetMenu.exitStageBtn().onClick = [this](bool m1) {
		if (m1) {
			armoryMenu.paused = armoryMenu.inStageMode = false;
			armoryMenu.stageSetMenu.full_reset();

			switch_menu(MenuType::STAGE_SELECT);
		}
		};

	for (int i = 0; i < UnitConfig::TOTAL_PLAYER_UNITS; i++) {
		armoryMenu.unitSelectionBtn(i).onClick = [i, this](bool isM1) {
			if (isM1) armoryMenu.start_dragging_unit(i);
			else {
				workshopMenu.setup_workshop_unit(i, armoryMenu.displayedGears[i]);
				switch_menu(MenuType::WORKSHOP_MENU);
			}
			};
	}
#pragma endregion

	workshopMenu.return_btn().onClick = [this](bool m1) {
		if (m1) {
			switch_menu(prevMenuType);
			armoryMenu.change_displayed_gear(workshopMenu.unitId, workshopMenu.unitGear);
		}
		};
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
	default: return menu;
	}

	std::cout << "Could not get Menu: " << (int)curMenuType << std::endl;
	return nullptr;
}
void PreparationState::start_stage_set(int stage, int set) {
	std::string jsonPath = std::format("configs/stage_data/stage_{}.json", stage);
	
	if (!std::filesystem::exists(jsonPath)) {
		std::cerr << "Error: Could not open file: " << jsonPath << std::endl;
		return;
	}
	if (armoryMenu.filledUnitSlots == -10) {
		std::cerr << "Error: loadout is empty. \n";
		return;
	}

	nextStateEnterData = std::make_unique<StageEnterData>(jsonPath, set, armoryMenu.slots);
	readyToEndState = true;
	std::cout << "starting stage #" << stage << std::endl;
}
void PreparationState::on_enter(OnStateEnterData* enterData) {
	if (auto prepData = dynamic_cast<PrepEnterData*>(enterData))
		enter_from_transition(prepData);
	else if (auto setData = dynamic_cast<StageSetPrepEnterData*>(enterData))
		enter_from_stage_set_completion(setData);
	else {
		std::cout << "Enter Data is not Prep Data" << std::endl;
		return;
	}
}
void PreparationState::enter_from_stage_set_completion(const StageSetPrepEnterData* setEnterData) {
	curMenuType = prevMenuType = MenuType::ARMORY_EQUIP;
}
void PreparationState::enter_from_transition(const PrepEnterData* prepData) {
	curMenuType = prepData->newMenuType;
	prevMenuType = prepData->prevMenuType;

	armoryMenu.inStageMode = false;
	armoryMenu.stageSetMenu.full_reset();

	menu = get_menu();
}

void PreparationState::on_exit() {
	/*
	* I'm yet to add functionalty to this, but will likely be something like 
	cleaning up Transitioning and unloading data
	*/
}