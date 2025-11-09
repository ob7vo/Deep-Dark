#pragma once
#include <iostream>
#include <memory>
#include "GameState.h"
#include "Camera.h"

struct StateManager {
	Camera& cam;

	std::array<GameState*, 3> gameStateMap;
	GameState* gameState = nullptr;
	GameState::Type stateType = GameState::Type::MAIN_MENU;
	GameState::Type nextStateType = GameState::Type::STAGE;

	bool switchingStates = false;

	StateManager(Camera& cam);

	void update(float deltaTime);
	void render();
	void handle_events(sf::Event);
	void switch_state(OnStateEnterData* newState);
};