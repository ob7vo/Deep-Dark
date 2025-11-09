#pragma once
#include "GameState.h"
#include "StartMenu.h"

class MainMenusState : public GameState {
	StartMenu startMenu;
public:
	MainMenusState(Camera& cam);
	~MainMenusState() = default;

	void update(float deltaTime) override;
	void render() override;
	void handle_events(sf::Event event) override;
	void on_enter(OnStateEnterData* enterData) override;
	void on_exit() override;

	void start_game();
};