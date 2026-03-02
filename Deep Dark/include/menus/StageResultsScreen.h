#pragma once
#include "Menu.h"

struct StageResultsScreen : public Menu<UI::StageUI::ResultsScreen::BTN_COUNT>{
	bool isVictorious = false;
	bool isNextStageSet = false;

	explicit StageResultsScreen(Camera& cam);
	~StageResultsScreen() final = default;

	void draw() final;
	void reset_positions() final;

	void setup_results_screen(bool victorious, bool isNextStageSet);
	inline Button& quitStageBtn() { return buttonManager.buttons[static_cast<int>(UI::StageUI::ResultsScreen::VS_ButtonIndex::QUIT_STAGE)]; }
	inline Button& nextStageSetBtn() { return buttonManager.buttons[static_cast<int>(UI::StageUI::ResultsScreen::VS_ButtonIndex::NEXT_STAGE_SET)]; }
};