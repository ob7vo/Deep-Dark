#include "WorkshopMenu.h"
#include "UnitData.h"
#include "UILayout.h"
#include <format>

using namespace UI::Workshop;

WorkshopMenu::WorkshopMenu(Camera& cam) : Menu(cam), statIcons(make_statIcons()),
statTexts(make_statTexts()) {
	unsigned int fontSize = cam.get_norm_font_size(unitNameText, UNIT_TEXT_SIZE);
	unitNameText.setCharacterSize(fontSize);
	unitDescText.setCharacterSize(fontSize);

	unsigned int fontSize2 = cam.get_norm_font_size(statTexts[0], STAT_TEXT_SIZE);
	for (int i = 0; i < STAT_ICONS; i++) {
		statTexts[i].setCharacterSize(fontSize2);
		statIcons[i].setOrigin((sf::Vector2f)statIcons[i].getTexture().getSize() * 0.5f);
	}
	 
	pause_btn().set_ui_params(UNIT_PAUSE_BTN_POS, UNIT_PAUSE_BTN_SIZE, uiPath + "pause.png", cam);
	pause_btn().onClick = [this](bool isM1) {if (isM1) paused = !paused; };
	return_btn().set_ui_params(RETURN_BTN_POS, RETURN_BTN_SIZE, uiPath + "return.png", cam);
	switchGearBtn().set_ui_params(UNIT_SWITCH_GEAR_BTN_POS, UNIT_SWITCH_GEAR_BTN_SIZE, workshopPath + "switch_gear.png", cam);
	switchGearBtn().onClick = [this](bool isM1) {if (isM1) switch_unit_gear(); };
	animationSpeedBtn().set_ui_params(UNIT_SPEED_BTN_POS, UNIT_SPEED_BTN_SIZE, workshopPath + "speed_up.png", cam);
	animationSpeedBtn().onClick = [this](bool isM1) {if (isM1) unitAnimSpeedIndex = (unitAnimSpeedIndex + 1) % 5; };

	const std::string path3 = uiPath + "return.png";

	animation_btn(0).set_ui_params({}, UNIT_ANIMATION_BTN_SIZE, workshopPath + "move_anim_btn.png", cam);
	animation_btn(1).set_ui_params({}, UNIT_ANIMATION_BTN_SIZE, workshopPath + "attack_anim_btn.png", cam);
	animation_btn(2).set_ui_params({}, UNIT_ANIMATION_BTN_SIZE, workshopPath + "idle_anim_btn.png", cam);
	animation_btn(3).set_ui_params({}, UNIT_ANIMATION_BTN_SIZE, workshopPath + "knockback_anim_btn.png", cam);
	animation_btn(4).set_ui_params({}, UNIT_ANIMATION_BTN_SIZE, workshopPath + "falling_anim_btn.png", cam);
	for (int i = 0; i < 5; i++) {
		animation_btn(i).onClick = [i, this](bool isM1) {
			if (!isM1) return;
			currentAnimation = static_cast<UnitAnimationState>(i);
			unitAnimations[currentAnimation].reset(unitSprite);
			unitHitboxes.clear();
			};
	}
}
void WorkshopMenu::setup_workshop_unit(int id, int gear) {
	const nlohmann::json unitJson = UnitData::get_unit_json(id, gear);

	unitHitboxes.clear();
	unitAnimations.clear();

	unitStats = UnitStats::player(unitJson);
	Animation::setup_unit_animation_map(unitJson, unitAnimations);
	for (auto& [state, anim] : unitAnimations) {
		anim.loops = true;
	}

	unitAnimations[currentAnimation].reset(unitSprite);
	set_stat_texts(unitJson);
}
void WorkshopMenu::set_stat_texts(const nlohmann::json& unitJson) {
	unitNameText.setString(unitJson["name"].get<std::string>());
	if (unitJson.contains("description"))
		unitDescText.setString(unitJson["description"].get<std::string>());
	else unitDescText.setString("No Description");

	stat_text("hp").setString(std::to_string(unitStats.maxHp));
	stat_text("speed").setString(std::to_string(unitStats.speed));
	stat_text("cost").setString(std::format("{} parts", unitStats.parts));
	stat_text("recharge_time").setString(std::format("{} s", unitStats.rechargeTime));
	stat_text("attack_time").setString(std::format("{} s", unitStats.attackTime));
	stat_text("sight_range").setString(std::format("{} m", unitStats.sightRange));
	stat_text("knockbacks").setString(std::format("{} knockbacks", unitStats.knockbacks));
	stat_text("hits").setString(std::to_string(unitStats.hits[0].dmg));
}
void WorkshopMenu::reset_positions() {
	return_btn().set_norm_pos(RETURN_BTN_POS, cam);
	pause_btn().set_norm_pos(UNIT_PAUSE_BTN_POS, cam);
	unitNameText.setPosition(cam.norm_to_pixels(UNIT_NAME_TEXT_POS));
	unitDescText.setPosition(cam.norm_to_pixels(UNIT_DESC_TEXT_POS));
	unitSprite.setPosition(cam.norm_to_pixels(UNIT_POS));

	sf::Vector2f iconPos = cam.norm_to_pixels(STAT_ICON_POS);
	sf::Vector2f inc = cam.norm_to_pixels(STAT_ICON_INCREMENT);
	sf::Vector2f offset = cam.norm_to_pixels(STAT_TEXT_OFFSET);

	for (int i = 0; i < STAT_ICONS; i++) {
		statIcons[i].setPosition(iconPos);
		statTexts[i].setPosition(iconPos + offset);
		iconPos += inc;
	}

	sf::Vector2f animPos = UNIT_ANIMATION_BTN_POS;

	for (int i = 0; i < 5; i++) {
		animation_btn(i).set_norm_pos(animPos, cam);
		animPos += UNIT_ANIMATION_BTN_INCREMENT;
	}
}

void WorkshopMenu::draw() {
	for (int i = 0; i < STAT_ICONS; i++) {
		cam.queue_ui_draw(&statIcons[i]);
		cam.queue_ui_draw(&statTexts[i]);
	}

	buttonManager.draw(cam);
	cam.queue_world_draw(&unitSprite);
	cam.queue_ui_draw(&unitNameText);
	cam.queue_ui_draw(&unitDescText);

	draw_unit_hurtbox();
	draw_unit_hitboxs();
}
void WorkshopMenu::draw_unit_hurtbox() {
	sf::Vector2f size = cam.norm_to_pixels(UNIT_ANIMATION_BTN_SIZE);
	sf::RectangleShape hurtbox(size);
	sf::Vector2f origin = { (!unitStats.is_player() ? 0.f : size.x) , size.y };

	hurtbox.setOrigin(origin);
	hurtbox.setPosition(unitSprite.getPosition());
	hurtbox.setFillColor(sf::Color(3, 252, 198, 128));

	cam.queue_temp_ui_draw(hurtbox);
}
void WorkshopMenu::draw_unit_hitboxs() {
	for (auto& hitbox : unitHitboxes)
		cam.queue_world_draw(&hitbox);
}
void WorkshopMenu::create_hitbox_visualizer() {
	auto hitIndex = static_cast<int>(unitHitboxes.size());
	const auto [minRange, maxRange]  = unitStats.get_hit_stats(hitIndex).attackRange;
	sf::Vector2f unitPos = unitSprite.getPosition();

	float width = maxRange - minRange;
	float height = UNIT_HITBOX_HEIGHTS[hitIndex];
	sf::RectangleShape shape({ width, height});

	float originX = unitStats.team == 1 ? 0 : width;
	float posX = unitPos.x + (minRange * static_cast<float>(unitStats.team));

	shape.setOrigin({ originX, height });
	shape.setFillColor(UNIT_HITBOX_COLORS[hitIndex]);
	shape.setPosition({ posX, unitPos.y });

	unitHitboxes.push_back(shape);
}

void WorkshopMenu::update(float dt) {
	if (paused) return;
	update_unit_animation(dt);
}
void WorkshopMenu::update_unit_animation(float deltaTime) {
	if (paused) return;

	float dt = deltaTime * UNIT_ANIMATION_SPEEDS[unitAnimSpeedIndex];
	int events = unitAnimations[currentAnimation].update(dt, unitSprite);
	
	if (events & FINAL_FRAME || unitAnimations[currentAnimation].currentFrame == 0)
		unitHitboxes.clear();
	else if (events & ATTACK) 
		create_hitbox_visualizer();
}

void WorkshopMenu::check_mouse_hover() {
	buttonManager.check_mouse_hover(cam.getMouseScreenPos());
}
bool WorkshopMenu::on_mouse_press(bool isM1) {
	return buttonManager.on_mouse_press(cam.getMouseScreenPos(), isM1);
}
bool WorkshopMenu::on_mouse_release(bool isM1) {
	return true;
}

void WorkshopMenu::switch_unit_gear() {
	unitGear = std::max((unitGear + 1) % 4, 1);
	setup_workshop_unit(unitId, unitGear);
}