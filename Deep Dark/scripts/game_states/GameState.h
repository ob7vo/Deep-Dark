#pragma once
#include "Camera.h"

struct OnStateEnterData;

class GameState {
public:
	GameState() = default;
	~GameState() = default;

	virtual void update(float deltaTime) = 0;
	virtual void render(Camera& cam) = 0;
	virtual void handle_events(sf::Event event) = 0;
	virtual void on_enter(OnStateEnterData* enterData) = 0;
	virtual void on_exit() = 0;
	virtual void update_ui(float deltaTime) {};

	enum class Type {
		MAIN_MENU = 0,
		UPGRADING = 1,
		STAGE = 2
	};
	virtual Type get_next_state() = 0;
};

struct OnStateEnterData {
	GameState::Type stateType = GameState::Type::MAIN_MENU;
	Camera& cam;

	OnStateEnterData(GameState::Type type, Camera& cam) : 
		stateType(type), cam(cam) {};
	virtual ~OnStateEnterData() = default;
};