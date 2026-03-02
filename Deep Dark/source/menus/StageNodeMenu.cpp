#include "pch.h"
#include "StageNodeMenu.h"
#include "Camera.h"
#include "UITextures.h"
#include "UILayout.h"
#include "Utils.h"

using namespace UI::StageSelect::StageNode;
using namespace Textures::UI;

StageNodeMenu::StageNodeMenu(Camera& cam) : Menu(cam) {
	cam.setup_sprite(MENU_POS, MENU_SIZE, menuSprite, t_menuBG1);
	cam.get_norm_font_size(stageNameText, STAGE_NAME_TEXT_SIZE);
	
	closeBtn().setup(CLOSE_BTN_POS, CLOSE_BTN_SIZE, cam, t_closeBtn);
	startStageBtn().setup(START_STAGE_BTN_POS, START_STAGE_BTN_SIZE, cam, t_startBtn);
	enterArmoryBtn().setup(ARMORY_BTN_POS, ARMORY_BTN_SIZE, cam, t_armoryBtn);
}

void StageNodeMenu::reset_positions() {
	menuSprite.setPosition(cam.norm_to_pixels(MENU_POS));
	stageNameText.setPosition(cam.norm_to_pixels(STAGE_NAME_TEXT_POS));

	closeBtn().set_pos(cam.norm_to_pixels(CLOSE_BTN_POS));
	startStageBtn().set_pos(cam.norm_to_pixels(START_STAGE_BTN_POS));
	enterArmoryBtn().set_pos(cam.norm_to_pixels(ARMORY_BTN_POS));
}

void StageNodeMenu::slide_menu_to_point(float t) {
	sf::Vector2f offset = SLIDE_AMOUNT * (clickable ? (1.f - t) : t); // at t=0, full offset. at t=1, no offset

	menuSprite.setPosition(cam.norm_to_pixels(MENU_POS + offset));
	stageNameText.setPosition(cam.norm_to_pixels(STAGE_NAME_TEXT_POS + offset));
	closeBtn().set_pos(cam.norm_to_pixels(CLOSE_BTN_POS + offset));
	startStageBtn().set_pos(cam.norm_to_pixels(START_STAGE_BTN_POS + offset));
	enterArmoryBtn().set_pos(cam.norm_to_pixels(ARMORY_BTN_POS + offset));
}
void StageNodeMenu::set_stage_name_text(int stageID) {
	nlohmann::json stageJson = StageConfig::getStageData(stageID);
	stageNameText.setString(stageJson["name"].get<std::string>());

	Printing::center_text(stageNameText);
}

void StageNodeMenu::draw() {
	cam.renderer.queue_ui_draw(&menuSprite);
	cam.renderer.queue_ui_draw(&stageNameText);
	buttonManager.draw(cam);
}
