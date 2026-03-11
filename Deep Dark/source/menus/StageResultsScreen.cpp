#include "pch.h"
#include "StageResultsScreen.h"
#include "StageUI.h"
#include "Camera.h"
#include "StageConfig.h"
#include "UILayout.h"

using namespace UI::StageUI::ResultsScreen;

StageResultsScreen::StageResultsScreen(Camera& cam) : Menu(cam) {

}

void StageResultsScreen::reset_positions() {
	quitStageBtn().set_pos(Screen::toPixels(QUIT_BTN_POS1));
	nextStageSetBtn().set_pos(Screen::toPixels(NEXT_STAGE_SET_BTN_POS));
}

void StageResultsScreen::draw() {
	cam.renderer.queue_ui_draw(&resultsMenu);
	cam.renderer.queue_ui_draw(&resultsText);
	buttonManager.draw(cam);
}
void StageResultsScreen::setup_results_screen(bool victorious, bool completedAllSets) {
	playerIsVictorious = victorious;
	clearedAllStageSets = completedAllSets;

	// Only show the quit_button, so move it into the middle
	if (!playerIsVictorious || (playerIsVictorious && completedAllSets)) {
		quitStageBtn().set_pos(Screen::toPixels(QUIT_BTN_POS2));
		nextStageSetBtn().visible = false;
	}
	// Show both the quit_button AND next_stage_set button
	else {
		quitStageBtn().set_pos(Screen::toPixels(QUIT_BTN_POS1));
		nextStageSetBtn().visible = true;
	}

	resultsText.setString((victorious ? "Stage Cleared." : "Defeat..."));
}
