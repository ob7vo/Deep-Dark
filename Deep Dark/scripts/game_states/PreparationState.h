#pragma once
#include "GameState.h"
#include "StageSelect.h"
#include "ArmoryMenu.h"

class PreparationState : public GameState {
	StageSelect stageSelect;
	ArmoryMenu armoryMenu;

	MenuBase* menu;
	MenuType curMenuType = MenuType::STAGE_SELECT;
	MenuType prevMenuType = MenuType::ARMORY_EQUIP;
public:
	PreparationState(Camera& cam);
	~PreparationState() = default;

	void update(float deltaTime) override;
	void render() override;
	void handle_events(sf::Event event) override;
	void on_enter(OnStateEnterData* enterData) override;
	void on_exit() override;
	void start_stage(int stage);

	inline MenuBase* get_menu() {
		cam.reset();

		switch (curMenuType) {
		case MenuType::STAGE_SELECT: 
			cam.change_lock(false);
			return &stageSelect;
		case MenuType::ARMORY_EQUIP: 
			cam.change_lock(true);
			return &armoryMenu;
		}

		return nullptr;
	}
};

struct PrepEnterData : public OnStateEnterData {
	MenuType openningMenuType = MenuType::STAGE_SELECT;
	MenuType prevMenuType = MenuType::STAGE_SELECT;

	PrepEnterData(MenuType first, MenuType prev) : openningMenuType(first), 
		prevMenuType(prev), OnStateEnterData(GameState::Type::PREP) { }
};