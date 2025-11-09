#include "ArmoryMenu.h"
#include "UILayout.h"

using namespace UI::ArmoryMenu;

ArmoryMenu::ArmoryMenu(Camera& cam) : Menu(cam) {
	sf::Vector2f normPos = FIRST_SLOT_POS;
	sf::Vector2f normInc = SLOT_INCREMENT;
	float startX = normPos.x;

	const std::string path = "sprites/defaults/empty_slot.png";
	std::vector<Button*> btns = {};
	for (int i = 0; i < 3; i++) {
		Button& btn = buttonManager.buttons[i];
		btn.set_ui_params(normPos, { 0.15f,0.15f }, path, cam);
		normPos.x += normInc.x;		
		btns.push_back(&btn);
	}
	
	const std::string path2 = "sprites/ui/slider.png";
	slider().set_ui_params(SLIDER_POS, SLIDER_SIZE, path2, cam);
	std::pair<float, float> bounds = { 100.f, 750.f };
	slider().setup(false, bounds, btns);
}

void ArmoryMenu::reset_positions() {
	sf::Vector2f pos = cam.norm_to_pixels(FIRST_SLOT_POS);
	sf::Vector2f inc = cam.norm_to_pixels(SLOT_INCREMENT);
	float startX = pos.x;

	for (int i = 0; i < 3; i++) {
		Button& btn = buttonManager.buttons[i];
		btn.set_pos(pos);
		pos.x += inc.x;
	}
}
void ArmoryMenu::update(float dt) {
	//check_mouse_hover();
	slider().update(cam.getMouseScreenPos(), dt);
}
void ArmoryMenu::draw() {
	buttonManager.draw(cam);
	cam.queue_ui_draw(&slider().sprite);
}
bool ArmoryMenu::on_mouse_press(bool isM1) {
	auto& mPos = cam.getMouseScreenPos();
	if (slider().check_mouse_hover(mPos))
		slider().onClick(isM1);

	buttonManager.on_mouse_press(mPos, isM1);
	return true;
}
bool ArmoryMenu::on_mouse_release(bool isM1) {
	if (isM1) slider().set_mouse_hold(false);
	return true;
}
void ArmoryMenu::check_mouse_hover() {
	auto& mPos = cam.getMouseScreenPos();

	slider().check_mouse_hover(mPos);
	buttonManager.check_mouse_hover(mPos);
}
