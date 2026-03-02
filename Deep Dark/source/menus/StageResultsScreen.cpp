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
	quitStageBtn().set_pos(cam.norm_to_pixels(QUIT_BTN_POS1));
	
}

void StageResultsScreen::draw() {

}
