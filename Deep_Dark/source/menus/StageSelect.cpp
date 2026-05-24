#include "pch.h"
#include "StageSelect.h"
#include "UILayout.h"
#include "Camera.h"
#include "UITextures.h"
#include "StageConfig.h"
#include "Easing.hpp"

#define DEBUG_STAGE_FOCUS 1

using namespace Textures::UI;
using namespace UI::StageSelect;
using namespace StageConfig;

constexpr float ZOOM_LEVEL_ON_NODE = 0.65f;
constexpr sf::Vector2f TRANSITION_CAM_POS_OFFEST = { 50.f, 0.f };

StageSelect::StageSelect(Camera& cam) : Menu(cam), stageNodeMenu(cam) {
	for (int i = 0; i < TOTAL_STAGES; i++) 
		stageNodeBtn(i).setup(STAGE_NODE_POS_ARR[i], STAGE_NODE_SIZE, t_stageNodeBtn);
	
	returnBtn().setup(RETURN_BTN_POS, RETURN_BTN_SIZE, t_returnBtn);
}

void StageSelect::reset_positions() {
	for (int i = 0; i < TOTAL_STAGES; i++)
		stageNodeBtn(i).set_pos(STAGE_NODE_POS_ARR[i]);

	returnBtn().set_pos(Screen::toPixels(RETURN_BTN_POS));
}

void StageSelect::update(float deltaTime) {
	if (isTransitioning) 
		transition_stage_node_menu(deltaTime);
}

void StageSelect::transition_stage_node_menu(float deltaTime) {
	// - zoom the camera in
// - slide in the StageNodeMenu

	transitionTime += deltaTime;
	float t = std::min(transitionTime / TRANSITION_DURATION, 1.f);
	t = Easing::easeOutQuad(t); // your ease function

	//if the menu is "paused" we are zooming in.
	float startingZoom = !zoomedInOnNode() ? ZOOM_LEVEL_ON_NODE : zoomLevelBeforeTransition;
	float targetZoom = zoomedInOnNode() ? ZOOM_LEVEL_ON_NODE : zoomLevelBeforeTransition;
	float newZoomLevel = std::lerp(startingZoom, targetZoom, t);
	cam.transform.set_zoom(newZoomLevel);
	
	sf::Vector2f targetPos = stageNodeBtn(selectedStage).get_pos() + TRANSITION_CAM_POS_OFFEST;
	sf::Vector2f newPos = camStartingPos + (targetPos - camStartingPos) * t;
	cam.transform.update_pos(newPos);

	stageNodeMenu.slide(t);
	slide(t);

	if (t >= 1.f) {
		isTransitioning = false;

		// If its NOT clickable, that means the menu was just closed
		if (!zoomedInOnNode()) {
			paused = false;
			cam.change_lock(false);
		}
	}
}
void StageSelect::slide(float t) {
	float slideOffset = DIST_OFFSCREEN * (!zoomedInOnNode() ? (1.f - t) : t);
	sf::Vector2f offsetUP = { 0.f, slideOffset };

	returnBtn().set_pos(Screen::toPixels(RETURN_BTN_POS - offsetUP));
}
void StageSelect::start_stage_node_menu_transition() {
	std::cout << "starting transition" << std::endl;
	isTransitioning = true;
	cam.change_lock(true);

	transitionTime = 0.f;
	camStartingPos = cam.getPosition();

	if (zoomedInOnNode()) { // opening
		zoomLevelBeforeTransition = cam.getZoomLevel(); // only capture when opening
	}
}

void StageSelect::draw(){
	if (paused) {
		cam.renderer.queue_world_draw(&stageNodeBtn(selectedStage).sprite);
		stageNodeMenu.draw();

		if (!isTransitioning) return;;
	}

	int end = TOTAL_STAGES;

	// StageNodes
	for (int i = 0; i < end; i++) {
		sf::FloatRect rect = stageNodeBtn(i).sprite.getGlobalBounds();
		cam.renderer.queue_world_draw(&stageNodeBtn(i).sprite, rect);
	}

	// The rest of the UI Buttons
	buttonManager.draw(cam, end);
}

bool StageSelect::on_mouse_press(bool isM1) {
	if (isTransitioning) return false;
	if (paused) return stageNodeMenu.on_mouse_press(isM1);

	// Stop at the last stageNode index as ONLY they use world Mouse Position
	auto worldMousePos = static_cast<sf::Vector2i>(cam.getMouseWorldPos());
	if (buttonManager.on_mouse_press(worldMousePos, isM1, 0, TOTAL_STAGES)) return true;

	// Noe check the rest of the UI Buttons
	return buttonManager.on_mouse_press(cam.getMouseScreenPos(), isM1, TOTAL_STAGES);
}
void StageSelect::check_mouse_hover() {
	if (isTransitioning) return;
	if (paused) {
		stageNodeMenu.check_mouse_hover();
		return;
	}

	auto worldMousePos = static_cast<sf::Vector2i>(cam.getMouseWorldPos());

// StageNodes
	buttonManager.check_mouse_hover(worldMousePos, 0, ButtonIndex::LAST_STAGE);

	// The rest of the UI Buttons
	buttonManager.check_mouse_hover(cam.getMouseScreenPos(), ButtonIndex::LAST_STAGE);
}

void StageSelect::on_enter() {
	cam.reset();
	cam.change_lock(false);
	cam.transform.update_pos(stageNodeBtn(selectedStage).get_pos());
}