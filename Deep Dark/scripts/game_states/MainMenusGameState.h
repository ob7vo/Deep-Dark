#pragma once
#include "GameState.h"
class MainMenusGameState : public GameState {
public:
	MainMenusGameState(Camera& cam);
	~MainMenusGameState() = default;

	void update(float deltaTime) override;
	void render(Camera& cam) override;
	void handle_events(sf::Event event) override;
	void on_enter(OnStateEnterData* enterData) override;
	void on_exit() override;
	void update_ui(float deltaTime) override;

	Type get_next_state() override;
};