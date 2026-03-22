#include "pch.h"
#include "StageUI.h"
#include "StageManager.h"
#include "UILayout.h"
#include "UITextures.h"
#include "Utils.h"

using enum Direction;

using namespace Textures::UI;
using namespace UI::StageUI;
using namespace Screen;

StageUI::StageUI(Camera& cam) : Menu(cam), pauseMenu(cam, *this), resultsScreen(cam){
	Visual::setupText(partsCountText, PARTS_TEXT_POS, TEXT_SIZE);
	Visual::setupText(bagUpgradeCostText, BAG_UPGRADE_COST_TEXT_POS, TEXT_SIZE);
	Visual::setupText(clearedChallengesText, CLEARED_CHALLENGES_TEXT_POS, TEXT_SIZE);

	create_buttons();
}
void StageUI::create_buttons() {
	pauseBtn().onClick = [this](bool m1) { if (m1) set_pause_state(true); };
	upgradeBagBtn().onClick = [this](bool m1) { if (m1) upgrade_bag(); };
	fireCannonBtn().onClick = [this](bool m1) { if (m1) fire_cannon(); };

	pauseBtn().setup(PAUSE_BTN_POS, PAUSE_BTN_SIZE, t_pauseBtn);
	upgradeBagBtn().setup(UPGRADE_BAG_BTN_POS, UPGRADE_BAG_BTN_SIZE, t_upgradeWalletBtn);
	fireCannonBtn().setup(FIRE_CANNON_BTN_POS, FIRE_CANNON_BTN_SIZE, t_fireCannonBtn);
}

void StageUI::reset_positions() {
	partsCountText.setPosition(toPixels(PARTS_TEXT_POS));
	bagUpgradeCostText.setPosition(toPixels(BAG_UPGRADE_COST_TEXT_POS));
	clearedChallengesText.setPosition(toPixels(CLEARED_CHALLENGES_TEXT_POS));

	pauseBtn().set_pos(toPixels(PAUSE_BTN_POS));
	upgradeBagBtn().set_pos(toPixels(UPGRADE_BAG_BTN_POS));
	fireCannonBtn().set_pos(toPixels(FIRE_CANNON_BTN_POS));

	pauseMenu.reset_positions();
}
void StageUI::slide(float t) {
	pauseBtn().set_pos(lerpOffscreen(PAUSE_BTN_POS, DIST_OFFSCREEN, t, Up));
	upgradeBagBtn().set_pos(lerpOffscreen(UPGRADE_BAG_BTN_POS, DIST_OFFSCREEN, t, Down));
	fireCannonBtn().set_pos(lerpOffscreen(FIRE_CANNON_BTN_POS, DIST_OFFSCREEN, t, Down));

	partsCountText.setPosition(lerpOffscreen(PARTS_TEXT_POS, DIST_OFFSCREEN, t, Up));
	bagUpgradeCostText.setPosition(lerpOffscreen(BAG_UPGRADE_COST_TEXT_POS, DIST_OFFSCREEN, t, Down));
	clearedChallengesText.setPosition(lerpOffscreen(CLEARED_CHALLENGES_TEXT_POS, DIST_OFFSCREEN, t, Down));
}
void StageUI::on_enter() {
	reset_positions();
	visible = clickable = true;

	paused = false;
	resultsScreen.visible = false;

	cam.reset();
	cam.change_lock(false);
}

 // Texts
void StageUI::set_texts() {
	partsCountText.setString(std::format("$0/{}", stageManager->wallet.partsCap));
	bagUpgradeCostText.setString(std::format("${}", stageManager->wallet.upgradeCost));

	clearedChallengesText.setString(
		std::format("Challenges Cleared: 0/{}", stageManager->challenges.size()));
	clearedChallengesText.setCharacterSize(16);
	clearedChallengesText.setFillColor(sf::Color::Yellow);
}
void StageUI::update_texts() {
	bagUpgradeCostText.setString(std::format("${}", stageManager->wallet.upgradeCost));
	partsCountText.setString(std::format("#{}/{}", stageManager->wallet.parts, stageManager->wallet.partsCap));


	// Challenges Text
	if (stageManager->clearedChallenges == stageManager->challenges.size())
		clearedChallengesText.setFillColor(sf::Color::Green);
	else 
		clearedChallengesText.setFillColor(sf::Color::Yellow);

	clearedChallengesText.setString(std::format("Challenges Cleared: {}/{}",
		stageManager->clearedChallenges, stageManager->challenges.size()));
}

void StageUI::draw() {
	if (resultsScreen.visible) {
		resultsScreen.draw();
		return;
	}
	if (!visible) return

	cam.renderer.queue_ui_draw(&partsCountText);
	cam.renderer.queue_ui_draw(&bagUpgradeCostText);
	cam.renderer.queue_ui_draw(&clearedChallengesText);

	buttonManager.draw(cam);

	if (!stageManager->can_fire_cannon())
		cam.renderer.draw_overlay(fireCannonBtn().sprite);
	if (stageManager->wallet.parts < stageManager->wallet.upgradeCost)
		cam.renderer.draw_overlay(upgradeBagBtn().sprite);

	// Draw pauseMenu last so it overlays
	if (paused) pauseMenu.draw();
}

bool StageUI::on_mouse_press(bool isM1) {
	if (!clickable) return resultsScreen.on_mouse_press(isM1);
		
	if (paused)
		return pauseMenu.on_mouse_press(isM1);
	else
		return buttonManager.on_mouse_press(cam.getMouseScreenPos(), isM1);
}
void StageUI::check_mouse_hover() {
	if (paused)
		pauseMenu.check_mouse_hover();
	else if (!clickable && resultsScreen.clickable)
		resultsScreen.check_mouse_hover();
	else
		buttonManager.check_mouse_hover(cam.getMouseScreenPos());
}

// Button Function
void StageUI::set_pause_state(bool pause) {
	paused = pause;
	cam.change_lock(pause);

	for (int i = 0; i < BTN_COUNT; i++) 
		buttonManager.buttons[i].sprite.setColor(sf::Color::White);
}
void StageUI::upgrade_bag() {
	if (stageManager->wallet.can_upgrade_bag(stageManager->stageRecorder))
		stageManager->wallet.upgrade_bag();

	update_texts();
}
void StageUI::fire_cannon() {
	stageManager->try_fire_cannon();
}

// Pause Menu
StagePauseMenu::StagePauseMenu(Camera& cam, StageUI& ui) : 
Menu(cam), stageUI(ui) {
	pauseText.setPosition(toPixels(PauseMenu::PAUSE_MENU_TEXT_POS));
	pauseText.setString("game is PAUSED baby");

	closeGameBtn().setup(PauseMenu::CLOSE_GAME_BTN_POS, PauseMenu::CLOSE_GAME_BTN_SIZE, t_returnBtn);
	closeMenuBtn().setup(PauseMenu::CLOSE_MENU_BTN_POS, PauseMenu::CLOSE_MENU_BTN_SIZE, t_closeBtn);

	closeGameBtn().onClick = [this](bool m1) { if (m1) close_game(); };
	closeMenuBtn().onClick = [this](bool m1) { if (m1) close_menu(); };

	Visual::setupUI(PauseMenu::PAUSE_MENU_POS, PauseMenu::PAUSE_MENU_SIZE, pauseMenuSprite, t_menuBG1);
}
void StagePauseMenu::reset_positions() {
	pauseText.setPosition(toPixels(PauseMenu::PAUSE_MENU_TEXT_POS));

	closeMenuBtn().set_pos(toPixels(PauseMenu::CLOSE_MENU_BTN_POS));
	closeGameBtn().set_pos(toPixels(PauseMenu::CLOSE_GAME_BTN_POS));

	pauseMenuSprite.setPosition(toPixels(PauseMenu::PAUSE_MENU_POS));
}
void StagePauseMenu::draw() {
	cam.renderer.darken_screen(0.5f);

	cam.renderer.queue_ui_draw(&pauseMenuSprite);
	cam.renderer.queue_ui_draw(&pauseText);

	buttonManager.draw(cam);
}
void StagePauseMenu::close_menu() { stageUI.set_pause_state(false); }
void StagePauseMenu::close_game() { cam.close_window(); }