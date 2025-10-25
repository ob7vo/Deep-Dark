#pragma once
#include "GameState.h"
#include "StageManager.h"

class StageGameState : public GameState {
	StageManager stageManager;
	Loadout loadout;
	StageUI stageUI;
public:
	StageGameState(Camera& cam);
	~StageGameState() = default;

	void update(float deltaTime) override;
	void render(Camera& cam) override;
	void handle_events(sf::Event event) override;
	void on_enter(OnStateEnterData* enterData) override;
	void on_exit() override;
	void update_ui(float deltaTime) override;

	Type get_next_state() override;
};

struct StageEnterData : public OnStateEnterData {
	const nlohmann::json stageJson;
	std::vector<std::string> loadoutSlots;

	StageEnterData(const nlohmann::json& stageJson, std::vector<std::string>& loadoutSlots,
		Camera& cam) : stageJson(stageJson), loadoutSlots(loadoutSlots), 
		OnStateEnterData(GameState::Type::STAGE, cam) {}
	~StageEnterData() = default;
};