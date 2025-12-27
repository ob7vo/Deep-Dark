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
	int stageId = 0;
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

	void quit_stage();
	void end_current_stage_set();
};

struct StageEnterData : public OnStateEnterData {
	std::string stageJsonPath;
	int stageSet;
	const std::array<ArmorySlot, 10>& slots;

	StageEnterData(const std::string& path, int set, const std::array<ArmorySlot, 10>& slots);

	~StageEnterData() override = default;
};