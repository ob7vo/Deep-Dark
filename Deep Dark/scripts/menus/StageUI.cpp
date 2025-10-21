#include "StageUI.h"
#include "StageManager.h"
using namespace UI::Stage;

StageUI::StageUI(Camera& cam, StageManager& manager) : Menu(cam),
stageManager(manager), pauseMenu(cam, *this){
	//	sf::Vector2f pos
	partsCountText.setPosition(cam.norm_to_pixels(PARTS_TEXT_POS));
	bagUpgradeCostText.setPosition(cam.norm_to_pixels(BAG_COST_TEXT_POS));
	clearedChallengesText.setPosition(cam.norm_to_pixels(CHALLENGES_TEXT_POS));

	create_buttons();
}
void StageUI::create_buttons() {
	pauseBtn().onClick = [this]() { pause(); };
	upgradeBagBtn().onClick = [this]() { upgrade_bag(); };
	fireCannonBtn().onClick = [this]() { fire_cannon(); };

	std::string texPath1 = "sprites/ui/stage_pause.png";
	std::string t = "configs/player_units/archdemon/archdemon_slot.png";

	pauseBtn().set_new_params(PAUSE_BTN_POS, PAUSE_BTN_SIZE, texPath1, cam);
	std::string texPath2 = "sprites/ui/upgrade_bag.png";
	upgradeBagBtn().set_new_params(BAG_BTN_POS, BAG_BTN_SIZE, texPath2, cam);
	std::string texPath3 = "sprites/ui/fire_cannon.png";
	fireCannonBtn().set_new_params(CANNON_BTN_POS, CANNON_BTN_SIZE, texPath3, cam);
}
void StageUI::draw() {
	cam.queue_ui_draw(&partsCountText);
	cam.queue_ui_draw(&bagUpgradeCostText);
	cam.queue_ui_draw(&clearedChallengesText);

	buttonManager.draw(cam);

	//if (!stageManager.can_fire_cannon())
		cam.queue_ui_draw(&fireCannonBtn().darkOverlay);
	//if (stageManager.parts < stageManager.bagUpgradeCost)
		cam.queue_ui_draw(&upgradeBagBtn().darkOverlay);

	if (paused) pauseMenu.draw();
}
void StageUI::register_click(sf::Vector2f mousePos) {
	if (paused)
		pauseMenu.register_click(mousePos);
	else
		buttonManager.register_click(mousePos);
}
void StageUI::check_mouse_hover(sf::Vector2f mousePos) {
	if (paused)
		pauseMenu.check_mouse_hover(mousePos);
	else
		buttonManager.check_mouse_hover(mousePos);
}
void StageUI::pause() {
	stageManager.pause();
	for (int i = 0; i < STAGE_UI_BUTTONS; i++) 
		buttonManager.buttons[i].sprite.setColor(sf::Color::White);
}
void StageUI::upgrade_bag() {
	if (stageManager.try_buy_upgrade_bag())
		stageManager.upgrade_bag();
}
void StageUI::fire_cannon() {
	stageManager.try_fire_cannon();
}

StagePauseMenu::StagePauseMenu(Camera& cam, StageUI& ui) : 
Menu(cam), stageUI(ui) {
	pauseText.setPosition(cam.norm_to_pixels(PAUSE_TEXT_POS));
	pauseText.setString("game is PAUSED baby");

	std::string texPath1 = "sprites/ui/close_game.png";
	closeGameBtn().set_new_params(CLOSE_GAME_BTN_POS, CLOSE_GAME_BTN_SIZE, texPath1, cam);
	std::string texPath2 = "sprites/ui/close_menu.png";
	closeMenuBtn().set_new_params(CLOSE_MENU_BTN_POS, CLOSE_MENU_BTN_SIZE, texPath2, cam);

	closeGameBtn().onClick = [this]() { close_game(); };
	closeMenuBtn().onClick = [this]() { close_menu(); };

	std::string texPath3 = "sprites/ui/pause_menu.png";
	cam.set_sprite_params(PAUSE_MENU_POS, PAUSE_MENU_SIZE,
		texPath3, pauseMenuTexture, pauseMenuSprite);
}
void StagePauseMenu::draw() {
	cam.draw_grey_screen(0.5f);

	cam.queue_ui_draw(&pauseMenuSprite);
	cam.queue_ui_draw(&pauseText);

	buttonManager.draw(cam);
}
void StagePauseMenu::close_menu() { stageUI.pause(); }
void StagePauseMenu::close_game() { cam.close_window(); }