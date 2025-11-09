#include "StateManager.h"
#include "StageGameState.h"
#include "MainMenusGameState.h"
#include "PreparationState.h"

StateManager::StateManager(Camera& cam) : cam(cam) {
	gameStateMap[0] = new MainMenusState(cam);
	gameStateMap[1] = new PreparationState(cam);
	gameStateMap[2] = new StageState(cam);
}
void StateManager::update(float deltaTime) {
	gameState->update(deltaTime);

	if (cam.dragging) cam.click_and_drag();
	else if (cam.has_velocity())
		cam.apply_velocity(deltaTime);
}
void StateManager::render() {
	gameState->render();
	cam.draw_all_ui();
}
void StateManager::handle_events(sf::Event event) {
	// add 'clicked' bool to gamestate base class to make it 
	// so camera cant register click on same frame
	gameState->handle_events(event);
	if (!gameState->clickedOnThisFrame())
		cam.handle_events(event);
	else
		gameState->reset_click_status();
}
void StateManager::switch_state(OnStateEnterData* newState) {
	GameState* oldState = gameState;

	gameState = gameStateMap[(int)newState->stateType];
	stateType = newState->stateType;
	gameState->on_enter(newState);

	if (oldState) {
		oldState->on_exit();
		oldState->reset();
	}

	cam.clear_queues();
}