#include "pch.h"
#include "StateManager.h"
#include "StageGameState.h"
#include "MainMenusGameState.h"
#include "PreparationState.h"
#include "DebugGameState.h"
#include "Utils.h"

StateManager::StateManager(Camera& cam) : 
	cam(cam)
	//textboxManager()
{
	gameStateMap[0] = std::make_unique<MainMenusState>(cam);
    gameStateMap[1] = std::make_unique<PreparationState>(cam);
    gameStateMap[2] = std::make_unique<StageState>(cam);
    //gameStateMap[3] = std::make_unique<DebugState>(cam);
	//textboxManager.create_text_box_callbacks(*this);
}

void StateManager::update(float deltaTime) {
	gameState->update(deltaTime);
	cam.update(deltaTime);
}
void StateManager::render() {
	gameState->render();
	cam.renderer.draw();
}

void StateManager::handle_events(sf::Event event) {
	gameState->handle_events(event);

	if (!gameState->clickedOnThisFrame())
		cam.handle_events(event);
	else
		gameState->reset_click_status();

	/*
	if (textboxManager.curTextBox) {
		textboxManager.type_in_text_box(event);
	}
	*/
}


void StateManager::switch_state(OnStateEnterData* newState) {
	GameState* oldState = gameState;

	gameState = gameStateMap[(int)newState->stateType].get();
	stateType = newState->stateType;
	gameState->on_enter(newState);

	if (oldState) {
		oldState->on_exit();
		oldState->reset();
	}

	cam.renderer.clear_queues();
}