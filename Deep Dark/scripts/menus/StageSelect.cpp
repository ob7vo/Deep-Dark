#include "StageSelect.h"
#include "UILayout.h"

using namespace UI::StageSelect;

StageSelect::StageSelect(Camera& cam) : Menu(cam){
  	std::string texPath = "sprites/ui/stage_select/stage_node.png";
	for (int i = 0; i < STAGES; i++) 
		stageBtn(i).set_world_params(STAGE_NODE_POS_ARR[i], STAGE_NODE_SIZE, texPath, cam);
	
	std::string texPath2 = "sprites/ui/stage_select/return_btn.png";
	returnBtn().set_ui_params(RETURN_BTN_POS, RETURN_BTN_SIZE, texPath2, cam);

	reset_positions();
}

void StageSelect::reset_positions() {
	for (int i = 0; i < STAGES; i++)
		stageBtn(i).sprite.setPosition(STAGE_NODE_POS_ARR[i]);

	returnBtn().sprite.setPosition(cam.norm_to_pixels(RETURN_BTN_POS));
}

void StageSelect::draw() {
	int end = STAGES;
	for (int i = 0; i < end; i++) {
		sf::FloatRect rect = stageBtn(i).sprite.getGlobalBounds();
		cam.queue_draw(&stageBtn(i).sprite, rect);
	}

	buttonManager.draw(cam, end);
}
void StageSelect::register_click() {
	int end = STAGES;
	sf::Vector2i mPos = static_cast<sf::Vector2i>(cam.get_mouse_world_position());
	buttonManager.register_click(mPos, 0, end);
	buttonManager.register_click(cam.get_mouse_screen_position(), end);
}
void StageSelect::check_mouse_hover() {
	int end = STAGES;
	sf::Vector2i mPos = static_cast<sf::Vector2i>(cam.get_mouse_world_position());
	buttonManager.check_mouse_hover(mPos, 0, end);
	buttonManager.check_mouse_hover(cam.get_mouse_screen_position(), end);
}