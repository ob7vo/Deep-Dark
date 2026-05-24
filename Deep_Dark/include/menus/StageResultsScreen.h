#pragma once
#include "Menu.h"

struct StageResultsScreen : public Menu<UI::StageUI::ResultsScreen::BTN_COUNT>{
	bool playerIsVictorious = false;
	bool clearedAllStageSets = false;
	bool isPracticing = false;

	sf::Sprite resultsMenu = sf::Sprite(defaultTexture);
	sf::Text resultsText = sf::Text(baseFont);

	explicit StageResultsScreen(Camera& cam);
	~StageResultsScreen() final = default;

	void draw() final;

	void reset_positions() final;
	void animate(float deltaTime);

	void setup_results_screen(bool victorious, bool completedAllSets, bool practicing);

	inline Button& quitStageBtn() { return buttonManager.buttons[static_cast<int>(UI::StageUI::ResultsScreen::VS_ButtonIndex::QUIT_STAGE)]; }
	inline Button& nextStageSetBtn() { return buttonManager.buttons[static_cast<int>(UI::StageUI::ResultsScreen::VS_ButtonIndex::NEXT_STAGE_SET)]; }
	inline Button& restartStageSetBtn() { return buttonManager.buttons[static_cast<int>(UI::StageUI::ResultsScreen::VS_ButtonIndex::RESTART_STAGE)]; }

	enum class Layout {
		ExitingStage,
		ProgressingThroughStage,
		Practicing
	};
};