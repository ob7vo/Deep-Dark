#include "pch.h"
#include "PreparationState.h"
#include "StageGameState.h"

PreparationState::PreparationState(Camera& cam) : 
	GameState(cam),
	stageSelect(cam), 
	armoryMenu(cam), 
	workshopMenu(cam)
{
	stageSelect.reset_positions();
	armoryMenu.reset_positions();
	workshopMenu.reset_positions();

	setup_button_functions();
}

void PreparationState::setup_button_functions() {
#pragma region StageSelect
	for (int stageID = 0; stageID < StageConfig::TOTAL_STAGES; stageID++)
		stageSelect.stageNodeBtn(stageID).onClick = [this, stageID](bool m1) {
		if (m1) {
			stageSelect.selectedStage = stageID;
			stageSelect.stageNodeBtn(stageID).sprite.setColor(sf::Color::White);
			std::cout << Printing::vec2(Screen::size) << std::endl;

			stageSelect.stageNodeMenu.clickable = stageSelect.paused = true;
			stageSelect.start_stage_node_menu_transition();
			stageSelect.stageNodeMenu.set_up_menu(stageID);
		}//start_stage_set(i, 0); 
		};

	stageSelect.stageNodeMenu.enterArmoryBtn().onClick = [this](bool m1) {
		if (m1) {
			armoryMenu.mode = ArmoryMenu::Mode::StagePreparation;
			armoryMenu.stagePreviewMenu.setup_menu(stageSelect.selectedStage);
			switch_menu(MenuType::ARMORY_EQUIP);
		}
		};
	stageSelect.stageNodeMenu.closeBtn().onClick = [this](bool isM1) {
		if (isM1) {
			stageSelect.stageNodeMenu.clickable = false;
			stageSelect.start_stage_node_menu_transition();
		}
		};
	stageSelect.stageNodeMenu.startStageBtn().onClick = [this](bool isM1) {
		if (isM1) {
			start_stage_phase(stageSelect.selectedStage, 0);
			stageSelect.paused = stageSelect.stageNodeMenu.clickable = false;
		}
		};
	for (int i = 0; i < StageConfig::MAX_PHASES; i++) {
		stageSelect.stageNodeMenu.startPracticingBtns(i).onClick = [this, i](bool isM1) {
			if (isM1) {
				start_stage_phase(stageSelect.selectedStage, i, true);
				stageSelect.paused = stageSelect.stageNodeMenu.clickable = false;
			}
			};
	}
#pragma endregion

#pragma region ArmoryMenu
	armoryMenu.returnBtn().onClick = [this](bool m1) {
		if (m1) {
			if (armoryMenu.mode == ArmoryMenu::Mode::Normal)
				switch_menu(prevMenuType);
			else
				exit_stage_preparation();
		}
		};

	armoryMenu.stagePreviewMenu.startStageBtn().onClick = [this](bool m1) {
		if (m1) start_stage_phase(armoryMenu.stagePreviewMenu.stageID, armoryMenu.stagePreviewMenu.stagePhase);
		};
	armoryMenu.stagePreviewMenu.exitStageBtn().onClick = [this](bool m1) {
		if (m1) {
			armoryMenu.paused = false;
			armoryMenu.mode = ArmoryMenu::Mode::Normal;
			armoryMenu.stagePreviewMenu.full_reset();

			switch_menu(MenuType::STAGE_SELECT);
		}
		};

	for (int id = 0; id < UnitConfig::TOTAL_PLAYER_UNITS; id++) {
		armoryMenu.inventorySlotBtn(id).onClick = [id, this](bool isM1) {
			if (isM1) armoryMenu.start_dragging_unit(id);
			else {
				workshopMenu.setup_workshop_unit(id);
				switch_menu(MenuType::WORKSHOP_MENU);
			}
			};
	}
#pragma endregion

	workshopMenu.return_btn().onClick = [this](bool m1) {
		if (m1) {
			switch_menu(prevMenuType);
			armoryMenu.update_display_of_unit(workshopMenu.unitId);
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
	std::cout << "getting menu: " << MenuBase::GetMenuName(curMenuType) << std::endl;

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
void PreparationState::switch_menu(MenuType newMenu) {
	prevMenuType = curMenuType;
	curMenuType = newMenu;

	if (menu != nullptr) menu->on_exit();

	menu = get_menu();
	menu->on_enter();
}

void PreparationState::start_stage_phase(int stageID, int phase, bool inPracticeMode) {
	std::string jsonPath = std::format("configs/stage_data/stage_{}.json", stageID);
	
	if (!std::filesystem::exists(jsonPath)) {
		std::cerr << "Error: Stage file does not exist: " << Printing::wrap(jsonPath) << std::endl;
		return;
	}
	if (armoryMenu.filledUnitSlots == 0) {
		std::cerr << "Error: loadout is empty. \n";
		return;
	}

	nextStateEnterData = std::make_unique<StageEnterData>(stageID, phase, armoryMenu.equipSlots, inPracticeMode);
	readyToEndState = true;
	std::cout << "starting stage #" << stageID << std::endl;
}
void PreparationState::exit_stage_preparation() {
	armoryMenu.mode = ArmoryMenu::Mode::Normal;
	switch_menu(MenuType::STAGE_SELECT);
}

void PreparationState::on_enter(OnStateEnterData* enterData) {
	if (auto prepData = dynamic_cast<PrepEnterData*>(enterData))
		on_generic_entry(prepData);
	else if (auto setData = dynamic_cast<EntryOnStagePhaseClear*>(enterData))
		on_enter_from_stage_phase_completion(setData);
	else {
		std::cout << "Enter Data is not Prep Data" << std::endl;
		return;
	}
}
void PreparationState::on_enter_from_stage_phase_completion(const EntryOnStagePhaseClear* phaseEnterData) {
	armoryMenu.stagePreviewMenu.setup_menu(phaseEnterData->stageID, phaseEnterData->nextPhase, &phaseEnterData->usedLoadout);
	armoryMenu.mode = ArmoryMenu::Mode::StagePreparation;

	switch_menu(MenuType::ARMORY_EQUIP);
}
void PreparationState::on_generic_entry(const PrepEnterData* prepData) {
	switch_menu(prepData->newMenuType);
	prevMenuType = prepData->prevMenuType;

	armoryMenu.mode = ArmoryMenu::Mode::Normal;
	armoryMenu.stagePreviewMenu.full_reset();

	menu = get_menu();
}

void PreparationState::on_exit() {
	/*
	* I'm yet to add functionalty to this, but will likely be something like 
	cleaning up Transitioning and unloading data
	*/
}