#pragma once
#include "Camera.h"
#include "TransitionID.h"
#include <memory>

struct OnStateEnterData;
const auto M1 = sf::Mouse::Button::Left;
const auto M2 = sf::Mouse::Button::Right;

class GameState {
public:
	Camera& cam;
	std::unique_ptr<OnStateEnterData> nextStateEnterData = nullptr;

	bool readyToEndState = false;
	bool clicked = false;

	TransitionID currentTransition;
	float transitionTimer = 0.f;
	float transitionTimeElapsed = 0.f;

	explicit GameState(Camera& cam) : cam(cam) {};
	virtual ~GameState() = default;

	virtual void update(float deltaTime) = 0;
	virtual void render() = 0;
	virtual void handle_events(sf::Event event) = 0;
	virtual void on_enter(OnStateEnterData* enterData) = 0;
	virtual void on_exit() = 0;
	virtual void update_ui(float deltaTime) {};
	virtual void check_transition(float deltaTime) {};

	enum class Type {
		MAIN_MENU = 0,
		PREP = 1,
		STAGE = 2
	};

	inline void reset() {
		nextStateEnterData = nullptr;
		readyToEndState = false;
	}
	// Sets the new transition (timer, enum, and resets timeElapsed)
	inline void start_transition(TransitionID newTransition) {
		transitionTimeElapsed = 0.f;
		currentTransition = newTransition;

		switch (newTransition) {
		case TransitionID::StageUIToResultsScreen:
			transitionTimer = 0.4f;
		case TransitionID::ResultsScreenToArmory:
		case TransitionID::ResultsScreenToStageSelect:
		case TransitionID::TestStageUISliding:
			transitionTimer = 0.5f;
		}
	}
	inline float updateTransitionTime(float deltaTime) {
		transitionTimeElapsed += deltaTime;
		return std::min((transitionTimeElapsed / transitionTimer), 1.f);
	}
	inline bool isTransitioning() const { return currentTransition == TransitionID::None; }

	inline OnStateEnterData* get_next_state() {
		return readyToEndState ? nextStateEnterData.get() : nullptr;
	}

	inline bool clickedOnThisFrame() const { return clicked; }
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

	explicit OnStateEnterData(GameState::Type type) : 
		stateType(type) {};
	virtual ~OnStateEnterData() = default;
};