#pragma once
#include <iostream>
#include <memory>
#include "GameState.h"
#include "Camera.h"

struct StateManager {
	Camera& cam;

	std::array<std::unique_ptr<GameState>, 3> gameStateMap;
	GameState* gameState = nullptr;
	GameState::Type stateType = GameState::Type::MAIN_MENU;
	GameState::Type nextStateType = GameState::Type::STAGE;
	bool switchingStates = false;

	sf::Text* curTextBox = nullptr;
	std::function<void(sf::Text*)> onTextSubmit;
	int textBoxIndex = 0;

	explicit StateManager(Camera& cam);

	void update(float deltaTime);
	void render();

	void handle_events(sf::Event event);
	void type_in_text_box(sf::Event event);
	void select_text_box(sf::Text* textBox, std::function<void(sf::Text*)> onTextSubmit);

	void switch_state(OnStateEnterData* newState);
};