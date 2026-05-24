#pragma once

enum class TransitionID {
	None = 0,
	OpenStageNodeMenu,
	StageUIToResultsScreen,
	ResultsScreenToArmory,
	ResultsScreenToStageSelect,
	TestStageUISliding
};

constexpr float getTransitionTimer(TransitionID transitionID) {
	switch (transitionID) {
	case TransitionID::StageUIToResultsScreen:
		return 0.55f;
	case TransitionID::ResultsScreenToArmory:
	case TransitionID::ResultsScreenToStageSelect:
		return 0.35f;
	default: return 0.f;
	}
}