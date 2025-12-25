#pragma once
#include "GameState.h"
#include "StageSelect.h"
#include "ArmoryMenu.h"
#include "WorkshopMenu.h"

struct PrepEnterData;
struct StageSetPrepEnterData;

class PreparationState : public GameState {
private:
	StageSelect stageSelect;
	ArmoryMenu armoryMenu;
	WorkshopMenu workshopMenu;

	MenuBase* menu = nullptr;
	MenuType curMenuType = MenuType::STAGE_SELECT;
	MenuType prevMenuType = MenuType::MAIN_MENU;
public:

	explicit PreparationState(Camera& cam);
	~PreparationState() override = default;
	void setup_button_functions();

	void update(float deltaTime) override;
	void render() override;
	void handle_events(sf::Event event) override;
	void on_enter(OnStateEnterData* enterData) override;
	void on_exit() override;
	void start_stage_set(int stage, int set);
	
	/// <summary> Spefically when clearing a stage and moving onto the next phase </summary>
	void enter_from_stage_set_completion(const StageSetPrepEnterData* setEnterData);
	/// <summary> Generic entry into Prep Phase </summary>
	void enter_from_transition(const PrepEnterData* prepData);

	MenuBase* get_menu();
	inline void switch_menu(MenuType newMenu) {
		prevMenuType = curMenuType;
		curMenuType = newMenu;

		menu = get_menu();
		if (menu) menu->reset_positions();
	}
};

struct PrepEnterData : public OnStateEnterData {
	MenuType prevMenuType = MenuType::STAGE_SELECT;
	MenuType newMenuType = MenuType::STAGE_SELECT;

	PrepEnterData(MenuType cur, MenuType prev) : OnStateEnterData(GameState::Type::PREP), 
		prevMenuType(prev), newMenuType(cur) { }
};
struct StageSetPrepEnterData : public OnStateEnterData {
	std::vector<int> usedUnitIDs;
	int stageId;
	int nextStageSet;

	StageSetPrepEnterData(const std::vector<int>& ids, int id, int set) : OnStateEnterData(GameState::Type::PREP),
		usedUnitIDs(ids), stageId(id), nextStageSet(set) {}
};