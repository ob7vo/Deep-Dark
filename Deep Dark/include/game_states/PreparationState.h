#pragma once
#include "GameState.h"
#include "StageSelect.h"
#include "ArmoryMenu.h"
#include "WorkshopMenu.h"

class PreparationState : public GameState {
	StageSelect stageSelect;
	ArmoryMenu armoryMenu;
	WorkshopMenu workshopMenu;

	MenuBase* menu;
	MenuType curMenuType = MenuType::STAGE_SELECT;
	MenuType prevMenuType = MenuType::MAIN_MENU;
public:
	PreparationState(Camera& cam);
	~PreparationState() = default;

	void update(float deltaTime) override;
	void render() override;
	void handle_events(sf::Event event) override;
	void on_enter(OnStateEnterData* enterData) override;
	void on_exit() override;
	void start_stage(int stage);

	MenuBase* get_menu();
	inline void switch_menu(MenuType newMenu) {
		prevMenuType = curMenuType;
		curMenuType = newMenu;

		menu = get_menu();
		if (menu) menu->reset_positions();
	}
};

struct PrepEnterData : public OnStateEnterData {
	MenuType openningMenuType = MenuType::STAGE_SELECT;
	MenuType prevMenuType = MenuType::STAGE_SELECT;

	PrepEnterData(MenuType first, MenuType prev) : openningMenuType(first), 
		prevMenuType(prev), OnStateEnterData(GameState::Type::PREP) { }
};