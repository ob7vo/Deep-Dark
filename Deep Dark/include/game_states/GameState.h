#pragma once
#include "Camera.h"
#include <memory>

struct OnStateEnterData;
const auto M1 = sf::Mouse::Button::Left;
const auto M2 = sf::Mouse::Button::Right;

class GameState {
protected:
	Camera& cam;
	std::unique_ptr<OnStateEnterData> nextStateEnterData = nullptr;
	bool readyToEndState = false;
	bool transitioning = false;
	bool clicked = false;

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
	inline bool clickedOnThisFrame() { return clicked; }
	inline void reset_click_status() { clicked = false; }
	inline bool is_M1(const sf::Event::MouseButtonPressed* click) const { 
		return click->button == M1; 
	}
	inline bool is_M1(const sf::Event::MouseButtonReleased* release) const {
		return release->button == M1;
	}
};

struct OnStateEnterData {
	GameState::Type stateType = GameState::Type::MAIN_MENU;

	OnStateEnterData(GameState::Type type) : 
		stateType(type) {};
	virtual ~OnStateEnterData() = default;
};