#include "pch.h"
#include "StageSelect.h"
#include "UILayout.h"
#include "Camera.h"

using namespace UI::StageSelect;

StageSelect::StageSelect(Camera& cam) : Menu(cam){
	for (int i = 0; i < STAGES; i++) 
		stageNodeBtn(i).setup(STAGE_NODE_POS_ARR[i], STAGE_NODE_SIZE, cam, TextureManager::t_stageNodeBtn);
	
	returnBtn().setup(RETURN_BTN_POS, RETURN_BTN_SIZE, cam, TextureManager::t_returnBtn);
	armoryBtn().setup(ARMORY_BTN_POS, ARMORY_BTN_SIZE, cam, TextureManager::t_armoryBtn);
}

void StageSelect::reset_positions() {
	for (int i = 0; i < STAGES; i++)
		stageNodeBtn(i).set_pos(STAGE_NODE_POS_ARR[i]);

	returnBtn().set_norm_pos(RETURN_BTN_POS, cam);
	armoryBtn().set_norm_pos(ARMORY_BTN_POS, cam);
}

void StageSelect::draw() {
	int end = STAGES;
	for (int i = 0; i < end; i++) {
		sf::FloatRect rect = stageNodeBtn(i).sprite.getGlobalBounds();
		cam.queue_world_draw(&stageNodeBtn(i).sprite, rect);
	}

	buttonManager.draw(cam, end);
}
bool StageSelect::on_mouse_press(bool isM1) {
	int end = STAGES;
	auto worldMousePos = static_cast<sf::Vector2i>(cam.getMouseWorldPos());

	if (buttonManager.on_mouse_press(worldMousePos, isM1, 0, end)) return true;
	return buttonManager.on_mouse_press(cam.getMouseScreenPos(), isM1, end);
}
void StageSelect::check_mouse_hover() {
	int end = STAGES;
	auto worldMousePos = static_cast<sf::Vector2i>(cam.getMouseWorldPos());

	buttonManager.check_mouse_hover(worldMousePos, 0, end);
	buttonManager.check_mouse_hover(cam.getMouseScreenPos(), end);
}