#pragma once
#include "GameState.h"
#include "StageManager.h"

struct ArmoryMenu;
struct ArmorySlot;

class StageState : public GameState {
private:
	StageManager stageManager;
	Loadout loadout;
	StageUI stageUI;
public:
	int stageId = 0;
	int curStageSet = 0;
	int stageSetCount = 1;

	explicit StageState(Camera& cam);
	~StageState() = default;

	void update(float deltaTime) override;
	void render() override;
	void handle_events(sf::Event event) override;
	void on_enter(OnStateEnterData* enterData) override;
	void on_exit() override;
	void update_ui(float deltaTime) override;

	void quit_stage();
	void end_stage_set();
};

struct StageEnterData : public OnStateEnterData {
	std::string stageJsonPath;
	int stageSet;
	const std::array<ArmorySlot, 10>& slots;

	StageEnterData(const std::string& path, int set, const ArmoryMenu& armory);
	StageEnterData(const std::string& path, int set, const std::array<ArmorySlot, 10>& slots) :
		OnStateEnterData(GameState::Type::STAGE), stageJsonPath(path), stageSet(set), slots(slots) {}

	~StageEnterData() = default;
};