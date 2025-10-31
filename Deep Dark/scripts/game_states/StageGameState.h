#pragma once
#include "GameState.h"
#include "StageManager.h"

class StageState : public GameState {
	StageManager stageManager;
	Loadout loadout;
	StageUI stageUI;
public:
	StageState(Camera& cam);
	~StageState() = default;

	void update(float deltaTime) override;
	void render() override;
	void handle_events(sf::Event event) override;
	void on_enter(OnStateEnterData* enterData) override;
	void on_exit() override;
	void update_ui(float deltaTime) override;

	void quit_stage();
};

struct StageEnterData : public OnStateEnterData {
	const nlohmann::json stageJson;
	std::vector<int> loadoutSlots;

	StageEnterData(const nlohmann::json& stageJson, std::vector<int>& loadoutSlots) 
		: stageJson(stageJson), loadoutSlots(loadoutSlots), 
		OnStateEnterData(GameState::Type::STAGE) {}
	~StageEnterData() = default;
};