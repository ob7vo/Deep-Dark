#pragma once
#include "Camera.h"
#include <memory>

struct OnStateEnterData;

class GameState {
protected:
	Camera& cam;
	std::unique_ptr<OnStateEnterData> nextStateEnterData = nullptr;
	bool readyToEndState = false;
	bool transitioning = false;
public:
	GameState(Camera& cam) : cam(cam) {};
	~GameState() = default;

	virtual void update(float deltaTime) = 0;
	virtual void render() = 0;
	virtual void handle_events(sf::Event event) = 0;
	virtual void on_enter(OnStateEnterData* enterData) = 0;
	virtual void on_exit() = 0;
	virtual void update_ui(float deltaTime) {};

	enum class Type {
		MAIN_MENU = 0,
		PREP = 1,
		STAGE = 2
	};

	inline void reset() {
		nextStateEnterData = nullptr;
		readyToEndState = false;
	}
	inline OnStateEnterData* get_next_state() {
		return readyToEndState ? nextStateEnterData.get() : nullptr;
	}
};

struct OnStateEnterData {
	GameState::Type stateType = GameState::Type::MAIN_MENU;

	OnStateEnterData(GameState::Type type) : 
		stateType(type) {};
	virtual ~OnStateEnterData() = default;
};