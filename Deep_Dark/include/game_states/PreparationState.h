#pragma once
#include "GameState.h"
#include "StageSelect.h"
#include "ArmoryMenu.h"
#include "WorkshopMenu.h"

struct PrepEnterData;
struct EntryOnStagePhaseClear;

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

	void start_stage_phase(int stage, int phase, bool isPracticing = false);
	void exit_stage_preparation();

	/// <summary> Specifically when clearing a stage and moving onto the next phase </summary>
	void on_enter_from_stage_phase_completion(const EntryOnStagePhaseClear* phaseClearData);
	/// <summary> Generic entry into Prep Phase </summary>
	void on_generic_entry(const PrepEnterData* prepData);

	MenuBase* get_menu();
	void switch_menu(MenuType newMenu);
};

// Entering any other way
struct PrepEnterData : public OnStateEnterData {
	MenuType prevMenuType = MenuType::STAGE_SELECT;
	MenuType newMenuType = MenuType::STAGE_SELECT;

	PrepEnterData(MenuType cur, MenuType prev) : OnStateEnterData(GameState::Type::PREP), 
		prevMenuType(prev), newMenuType(cur) { }
};
// For when you enter from completing a stage
struct EntryOnStagePhaseClear : public OnStateEnterData {
	Loadout& usedLoadout;
	int stageID;
	int nextPhase;

	EntryOnStagePhaseClear(Loadout& loadout, int ID, int phase) : OnStateEnterData(GameState::Type::PREP),
		usedLoadout(loadout), stageID(ID), nextPhase(phase) {}
};