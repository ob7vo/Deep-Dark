#include "StageSelect.h"
#include "UILayout.h"

using namespace UI::StageSelect;

StageSelect::StageSelect(Camera& cam) : Menu(cam){
  	std::string texPath = "sprites/ui/stage_select/stage_node.png";
	for (int i = 0; i < STAGES; i++) 
		stageBtn(i).setup_world(STAGE_NODE_POS_ARR[i], STAGE_NODE_SIZE, texPath, cam);
	
	std::string texPath2 = "sprites/ui/stage_select/return_btn.png";
	returnBtn().set_ui_params(RETURN_BTN_POS, RETURN_BTN_SIZE, texPath2, cam);

	std::string texPath3 = "sprites/ui/stage_select/armory_btn.png";
	armoryBtn().set_ui_params(ARMORY_BTN_POS, ARMORY_BTN_SIZE, texPath3, cam);
	//reset_positions();
}

void StageSelect::reset_positions() {
	for (int i = 0; i < STAGES; i++)
		stageBtn(i).set_pos(STAGE_NODE_POS_ARR[i]);

	returnBtn().set_norm_pos(RETURN_BTN_POS, cam);
	armoryBtn().set_norm_pos(ARMORY_BTN_POS, cam);
}

void StageSelect::draw() {
	int end = STAGES;
	for (int i = 0; i < end; i++) {
		sf::FloatRect rect = stageBtn(i).sprite.getGlobalBounds();
		cam.queue_world_draw(&stageBtn(i).sprite, rect);
	}

	buttonManager.draw(cam, end);
}
bool StageSelect::on_mouse_press(bool isM1) {
	int end = STAGES;
	sf::Vector2i mPos = static_cast<sf::Vector2i>(cam.getMouseWorldPos());

	if (buttonManager.on_mouse_press(mPos, isM1, 0, end)) return true;
	return buttonManager.on_mouse_press(cam.getMouseScreenPos(), isM1, end);
}
void StageSelect::check_mouse_hover() {
	int end = STAGES;
	sf::Vector2i mPos = static_cast<sf::Vector2i>(cam.getMouseWorldPos());
	buttonManager.check_mouse_hover(mPos, 0, end);
	buttonManager.check_mouse_hover(cam.getMouseScreenPos(), end);
}