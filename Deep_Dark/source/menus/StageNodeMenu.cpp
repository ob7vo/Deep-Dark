#include "pch.h"
#include "StageNodeMenu.h"
#include "Camera.h"
#include "UITextures.h"
#include "UILayout.h"
#include "StageSaveData.h"
#include "Utils.h"

using namespace UI::StageSelect::StageNode;
using namespace Textures::UI;

StageNodeMenu::StageNodeMenu(Camera& cam) : Menu(cam) {
	Visual::setupUI(MENU_POS, MENU_SIZE, menuSprite, t_menuBG1);
	Screen::setFontSize(stageNameText, STAGE_NAME_TEXT_SIZE);
	
	closeBtn().setup(CLOSE_BTN_POS, CLOSE_BTN_SIZE, t_closeBtn);
	startStageBtn().setup(START_STAGE_BTN_POS, START_STAGE_BTN_SIZE, t_startBtn);
	enterArmoryBtn().setup(ARMORY_BTN_POS, ARMORY_BTN_SIZE, t_armoryBtn);

	for (size_t i = 0; i < StageConfig::MAX_PHASES; i++)
		startPracticingBtns(i).setup({}, PRACTICE_SET_BTNS_SIZE, t_stageNodeBtn);
}

void StageNodeMenu::reset_positions() {
	menuSprite.setPosition(Screen::toPixels(MENU_POS));
	stageNameText.setPosition(Screen::toPixels(STAGE_NAME_TEXT_POS));

	closeBtn().set_pos(Screen::toPixels(CLOSE_BTN_POS));
	startStageBtn().set_pos(Screen::toPixels(START_STAGE_BTN_POS));
	enterArmoryBtn().set_pos(Screen::toPixels(ARMORY_BTN_POS));
}

void StageNodeMenu::slide(float t) {
	sf::Vector2f offset = { TRANSITION_SLIDE_DISTANCE * (clickable ? (1.f - t) : t), 0.f }; // at t=0, full offset. at t=1, no offset

	menuSprite.setPosition(Screen::toPixels(MENU_POS + offset));
	stageNameText.setPosition(Screen::toPixels(STAGE_NAME_TEXT_POS + offset));
	closeBtn().set_pos(Screen::toPixels(CLOSE_BTN_POS + offset));
	startStageBtn().set_pos(Screen::toPixels(START_STAGE_BTN_POS + offset));
	enterArmoryBtn().set_pos(Screen::toPixels(ARMORY_BTN_POS + offset));

	if (practicableStagePhases > 1) {
		// Need to gwt the positioning again to slide the practice set buttons
		sf::Vector2f center = Screen::toPixels(PRACTICE_SET_BTNS_CENTER_POS);
		sf::Vector2f spacing = Screen::getSpacing(PRACTICE_SET_BTNS_SIZE, PRACTICE_SET_BTNS_SPACING);
		sf::Vector2f left = center - (spacing * ((float)practicableStagePhases - 1.f) * 0.5f);

		for (int i = 0; i < StageConfig::MAX_PHASES; i++) {
			startPracticingBtns(i).set_pos((left + (spacing * (float)i) + Screen::toPixels(offset)));
		}
	}
}
void StageNodeMenu::set_up_menu(int stageID) {
	nlohmann::json stageJson = StageConfig::getStageJson(stageID);
	stageNameText.setString(stageJson["name"].get<std::string>());

	practicableStagePhases = StageSaveData::GetHighestPhaseCleared(stageID) + 1;

	// The practice buttons only show if there are more than 1, as there's no point in "practicing" just the first set
	for (size_t i = 0; i < StageConfig::MAX_PHASES; i++) {
		startPracticingBtns(i).visible = (practicableStagePhases > 1) && (i < practicableStagePhases);
	}
	
	Printing::center_text(stageNameText);
}
void StageNodeMenu::draw() {
	cam.renderer.queue_ui_draw(&menuSprite);
	cam.renderer.queue_ui_draw(&stageNameText);
	buttonManager.draw(cam);
}
