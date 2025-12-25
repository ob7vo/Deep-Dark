#include "pch.h"
#include "StageUI.h"
#include "StageManager.h"
#include "UILayout.h"
#include "UITextures.h"

using namespace Textures::UI;
using namespace UI::StageUI;

StageUI::StageUI(Camera& cam) : Menu(cam), pauseMenu(cam, *this){
	//	sf::Vector2f pos
	partsCountText.setPosition(cam.norm_to_pixels(PARTS_TEXT_POS));
	bagUpgradeCostText.setPosition(cam.norm_to_pixels(BAG_COST_TEXT_POS));
	clearedChallengesText.setPosition(cam.norm_to_pixels(CHALLENGES_TEXT_POS));

	create_buttons();
}
void StageUI::create_buttons() {
	pauseBtn().onClick = [this](bool m1) { if (m1) pause(); };
	upgradeBagBtn().onClick = [this](bool m1) { if (m1) upgrade_bag(); };
	fireCannonBtn().onClick = [this](bool m1) { if (m1) fire_cannon(); };

	pauseBtn().setup(PAUSE_BTN_POS, PAUSE_BTN_SIZE, cam, t_pauseBtn);
	upgradeBagBtn().setup(BAG_BTN_POS, BAG_BTN_SIZE, cam, t_upgradeWalletBtn);
	fireCannonBtn().setup(CANNON_BTN_POS, CANNON_BTN_SIZE, cam, t_fireCannonBtn);
}
void StageUI::reset_positions() {
	partsCountText.setPosition(cam.norm_to_pixels(PARTS_TEXT_POS));
	bagUpgradeCostText.setPosition(cam.norm_to_pixels(BAG_COST_TEXT_POS));
	clearedChallengesText.setPosition(cam.norm_to_pixels(CHALLENGES_TEXT_POS));

	pauseBtn().sprite.setPosition(cam.norm_to_pixels(PAUSE_BTN_POS));
	upgradeBagBtn().sprite.setPosition(cam.norm_to_pixels(BAG_COST_TEXT_POS));
	fireCannonBtn().sprite.setPosition(cam.norm_to_pixels(CANNON_BTN_POS));

	pauseMenu.reset_positions();
}
void StageUI::draw() {
	cam.queue_ui_draw(&partsCountText);
	cam.queue_ui_draw(&bagUpgradeCostText);
	cam.queue_ui_draw(&clearedChallengesText);

	buttonManager.draw(cam);

	if (!stageManager->can_fire_cannon())
		cam.draw_overlay(fireCannonBtn().sprite);
	if (stageManager->wallet.parts < stageManager->wallet.upgradeCost)
		cam.draw_overlay(upgradeBagBtn().sprite);

	if (paused) pauseMenu.draw();
}
bool StageUI::on_mouse_press(bool isM1) {
	if (paused)
		return pauseMenu.on_mouse_press(isM1);
	else
		return buttonManager.on_mouse_press(cam.getMouseScreenPos(), isM1);
}
void StageUI::check_mouse_hover() {
	if (paused)
		pauseMenu.check_mouse_hover();
	else
		buttonManager.check_mouse_hover(cam.getMouseScreenPos());
}
void StageUI::pause() {
	stageManager->pause();
	for (int i = 0; i < BTN_COUNT; i++) 
		buttonManager.buttons[i].sprite.setColor(sf::Color::White);
}
void StageUI::upgrade_bag() {
	if (stageManager->wallet.try_buy_upgrade_bag(stageManager->stageRecorder))
		stageManager->upgrade_bag();
}
void StageUI::fire_cannon() {
	stageManager->try_fire_cannon();
}

StagePauseMenu::StagePauseMenu(Camera& cam, StageUI& ui) : 
Menu(cam), stageUI(ui) {
	pauseText.setPosition(cam.norm_to_pixels(PAUSE_TEXT_POS));
	pauseText.setString("game is PAUSED baby");

	closeGameBtn().setup(PauseMenu::CLOSE_GAME_BTN_POS, PauseMenu::CLOSE_GAME_BTN_SIZE, cam, t_returnBtn);
	closeMenuBtn().setup(PauseMenu::CLOSE_MENU_BTN_POS, PauseMenu::CLOSE_MENU_BTN_SIZE, cam, t_closeBtn);

	closeGameBtn().onClick = [this](bool m1) { if (m1) close_game(); };
	closeMenuBtn().onClick = [this](bool m1) { if (m1) close_menu(); };

	cam.setup_sprite(PauseMenu::PAUSE_MENU_POS, PauseMenu::PAUSE_MENU_SIZE, pauseMenuSprite, t_menuBG1);
}
void StagePauseMenu::reset_positions() {
	pauseText.setPosition(cam.norm_to_pixels(PAUSE_TEXT_POS));

	closeMenuBtn().sprite.setPosition(cam.norm_to_pixels(PauseMenu::CLOSE_MENU_BTN_POS));
	closeGameBtn().sprite.setPosition(cam.norm_to_pixels(PauseMenu::CLOSE_GAME_BTN_POS));

	pauseMenuSprite.setPosition(cam.norm_to_pixels(PauseMenu::PAUSE_MENU_POS));
}
void StagePauseMenu::draw() {
	cam.darken_screen(0.5f);

	cam.queue_ui_draw(&pauseMenuSprite);
	cam.queue_ui_draw(&pauseText);

	buttonManager.draw(cam);
}
void StagePauseMenu::close_menu() { stageUI.pause(); }
void StagePauseMenu::close_game() { cam.close_window(); }