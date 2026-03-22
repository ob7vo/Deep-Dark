#include "pch.h"
#include "StageResultsScreen.h"
#include "StageUI.h"
#include "Camera.h"
#include "StageConfig.h"
#include "UITextures.h"
#include "UILayout.h"

using namespace UI::StageUI::ResultsScreen;

StageResultsScreen::Layout get_results_layout(bool victorious, bool completedAllSets, bool practicing) {
	if (practicing) return StageResultsScreen::Layout::Practicing;
	if (!victorious || completedAllSets) return StageResultsScreen::Layout::ExitingStage;
	return StageResultsScreen::Layout::ProgressingThroughStage;
}

StageResultsScreen::StageResultsScreen(Camera& cam) : Menu(cam) {
	Visual::setupUI(MENU_POS, MENU_SIZE, resultsMenu, Textures::UI::t_menuBG1);
	Visual::setupText(resultsText, VICTORY_TEXT_POS, VICTORY_TEXT_SIZE);

	quitStageBtn().setup(QUIT_BTN_POS_LEFT, STAGE_BTNS_SIZE, Textures::UI::t_quitStageBtn);
	nextStageSetBtn().setup(SECOND_STAGE_BTN_POS, STAGE_BTNS_SIZE, Textures::UI::t_startNextStageSetBtn);
	restartStageSetBtn().setup(SECOND_STAGE_BTN_POS, STAGE_BTNS_SIZE, Textures::UI::t_restartStageSetBtn);
}

void StageResultsScreen::reset_positions() {
	quitStageBtn().set_pos(Screen::toPixels(QUIT_BTN_POS_LEFT));
	nextStageSetBtn().set_pos(Screen::toPixels(SECOND_STAGE_BTN_POS));
	restartStageSetBtn().set_pos(Screen::toPixels(SECOND_STAGE_BTN_POS));
}

void StageResultsScreen::draw() {
	cam.renderer.queue_ui_draw(&resultsMenu);
	cam.renderer.queue_ui_draw(&resultsText);
	buttonManager.draw(cam);
}
void StageResultsScreen::animate(float dt) {

}

void StageResultsScreen::setup_results_screen(bool victorious, bool completedAllSets, bool practicing) {
	playerIsVictorious = victorious;
	clearedAllStageSets = completedAllSets;
	isPracticing = practicing;
	Layout layout = get_results_layout(victorious, completedAllSets, practicing);

	switch (layout) {
	case Layout::Practicing: {
		std::cout << "In Practicing Layout" << std::endl;
		quitStageBtn().set_pos(Screen::toPixels(QUIT_BTN_POS_LEFT));
		restartStageSetBtn().visible = true;
		nextStageSetBtn().visible = false;
		break;
	}
	case Layout::ExitingStage: {
		std::cout << "In ExitingStage Layout" << std::endl;
		quitStageBtn().set_pos(Screen::toPixels(QUIT_BTN_POS_CENTER));
		nextStageSetBtn().visible = false;
		restartStageSetBtn().visible = false;
		break;
	}
	case Layout::ProgressingThroughStage: {
		std::cout << "In ProgressingThroughStage Layout" << std::endl;
		quitStageBtn().set_pos(Screen::toPixels(QUIT_BTN_POS_LEFT));
		nextStageSetBtn().visible = true;
		restartStageSetBtn().visible = false;
		break;
	}
	}

	resultsText.setString((victorious ? "Stage Cleared." : "Defeat..."));
}
