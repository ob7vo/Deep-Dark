#include "pch.h"
#include "StageSelect.h"
#include "UILayout.h"
#include "Camera.h"

#define DEBUG_STAGE_FOCUS 1

using namespace UI::StageSelect;

StageSelect::StageSelect(Camera& cam) : Menu(cam){
	for (int i = 0; i < STAGES; i++) 
		stageNodeBtn(i).setup(STAGE_NODE_POS_ARR[i], STAGE_NODE_SIZE, cam, TextureManager::t_stageNodeBtn);
	
	returnBtn().setup(RETURN_BTN_POS, RETURN_BTN_SIZE, cam, TextureManager::t_returnBtn);
	enterArmoryBtn().setup(ARMORY_BTN_POS, ARMORY_BTN_SIZE, cam, TextureManager::t_armoryBtn);
}

void StageSelect::reset_positions() {
	for (int i = 0; i < STAGES; i++)
		stageNodeBtn(i).set_pos(STAGE_NODE_POS_ARR[i]);

	returnBtn().set_norm_pos(RETURN_BTN_POS, cam);
	enterArmoryBtn().set_norm_pos(ARMORY_BTN_POS, cam);
}

void StageSelect::draw() {
	int end = STAGES;

	// StageNodes
	for (int i = 0; i < end; i++) {
		sf::FloatRect rect = stageNodeBtn(i).sprite.getGlobalBounds();
		cam.queue_world_draw(&stageNodeBtn(i).sprite, rect);
	}

	// The rest of the UI Buttons
	buttonManager.draw(cam, end);
}
bool StageSelect::on_mouse_press(bool isM1) {
	int end = STAGES;
	auto worldMousePos = static_cast<sf::Vector2i>(cam.getMouseWorldPos());

	// StageNodes (in world space)
	if (buttonManager.on_mouse_press(worldMousePos, isM1, 0, end)) return true;

	// The rest of the UI Buttons
	return buttonManager.on_mouse_press(cam.getMouseScreenPos(), isM1, end);
}
void StageSelect::check_mouse_hover() {
	int end = STAGES;
	auto worldMousePos = static_cast<sf::Vector2i>(cam.getMouseWorldPos());

// StageNodes
#if DEBUG_STAGE_FOCUS
	for (int i = 0; i < end; i++)
		if (buttonManager.buttons[i].check_mouse_hover(worldMousePos)) {
			selectedStage = i;
			break;
		}
#else
	buttonManager.check_mouse_hover(worldMousePos, 0, end);
#endif

	// The rest of the UI Buttons
	buttonManager.check_mouse_hover(cam.getMouseScreenPos(), end);
}