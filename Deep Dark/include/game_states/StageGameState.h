#pragma once
#include "GameState.h"
#include "StageManager.h"

struct ArmoryMenu;
struct ArmorySlot;

class StageState : public GameState {
private:
	StageUI stageUI;
	Loadout loadout;
	StageManager stageManager;
public:
	int curStageID = 0;
	int curStageSet = 0;
	int stageSetCount = 1;

	explicit StageState(Camera& cam);
	~StageState() override = default;

	void update(float deltaTime) override;
	void render() override;
	void on_enter(OnStateEnterData* enterData) override;
	void on_exit() override;
	void update_ui(float deltaTime) override;

	void handle_events(sf::Event event) override;

	void handle_stage_victory();
	/// <summary>
	/// Needed to fully exit the stage. Only called via result screen quit button, or
	/// from the quit button in the armory when preparing for the next stage set
	/// </summary>
	void quit_stage();
	void end_current_stage_set(bool playerWon);
	void enter_next_stage_set();
};

struct StageEnterData : public OnStateEnterData {
	std::string stageJsonPath;
	int stageSet;
	const std::array<ArmorySlot, 10>& slots;

	StageEnterData(const std::string& path, int set, const std::array<ArmorySlot, 10>& slots);

	~StageEnterData() override = default;
};