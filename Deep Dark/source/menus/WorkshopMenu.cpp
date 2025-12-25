#include "pch.h"
#include "WorkshopMenu.h"
#include "UnitData.h"
#include "Camera.h"
#include "UILayout.h"
#include "UITextures.h"

using namespace Textures::UI;
using namespace UI::Workshop;
using namespace UI::Colors;

WorkshopMenu::WorkshopMenu(Camera& cam) : Menu(cam), statIcons(make_statIcons()),
statTexts(make_statTexts()) {
	unsigned int fontSize = cam.get_norm_font_size(unitNameText, UNIT_TEXT_SIZE);
	unitNameText.setCharacterSize(fontSize);
	unitDescText.setCharacterSize(fontSize);

	unsigned int fontSize2 = cam.get_norm_font_size(statTexts[0], STAT_TEXT_SIZE);
	for (int i = 0; i < STAT_ICONS; i++) {
		statTexts[i].setCharacterSize(fontSize2);
		statIcons[i].setTextureRect(r_workshopStatsIcons[i]);
		statIcons[i].setOrigin(statIcons[i].getLocalBounds().size * 0.5f);
	}
	 
	pause_btn().setup(UNIT_PAUSE_BTN_POS, UNIT_PAUSE_BTN_SIZE, cam, t_pauseBtn);
	return_btn().setup(RETURN_BTN_POS, RETURN_BTN_SIZE, cam, t_returnBtn);
	switchGearBtn().setup(UNIT_SWITCH_GEAR_BTN_POS, UNIT_SWITCH_GEAR_BTN_SIZE, cam, t_switchGearBtn);
	animationSpeedBtn().setup(UNIT_SPEED_BTN_POS, UNIT_SPEED_BTN_SIZE, cam, t_speedUpBtn);

	pause_btn().onClick = [this](bool isM1) {if (isM1) paused = !paused; };
	switchGearBtn().onClick = [this](bool isM1) {if (isM1) switch_unit_gear(); };
	animationSpeedBtn().onClick = [this](bool isM1) {if (isM1) unitAnimSpeedIndex = (unitAnimSpeedIndex + 1) % 5; };

	for (int i = 0; i < 5; i++) {
		// Setting the positions for animPlayer Btns takes a lot fo lines, so its done in reset_positions()
		animation_btn(i).setup({}, UNIT_ANIMATION_BTN_SIZE, cam, t_workshopAnimBtns, r_workshopAnimBtns[i]);
		animation_btn(i).onClick = [i, this](bool isM1) {
			if (!isM1) return;
			auto newAnimState = static_cast<UnitAnimationState>(i);
			unitHitboxes.clear();

			if (newAnimState != currentAnimState)
				unitAnimPlayer.start(&unitAnimations[newAnimState], unitSprite);
			else unitAnimPlayer.reset(unitSprite);

			currentAnimState = newAnimState;
			};
	}
}

void WorkshopMenu::setup_workshop_unit(int id, int gear) {
	unitId = id;
	unitGear = gear;
	highestGear = UnitData::getMaxGear(id);

	const nlohmann::json unitJson = UnitData::createUnitJson(id, gear);

	unitHitboxes.clear();
	unitAnimTextures.clear();
	unitAnimations.clear();

	unitStats = UnitStats::create_player(unitJson);
	AnimationClip::setup_unit_animation_map(unitJson, unitAnimations, unitAnimTextures);

	for (auto& [state, animPlayer] : unitAnimations) animPlayer.loops = true;
	currentAnimState = UnitAnimationState::MOVE;

	unitAnimPlayer.start(&unitAnimations[currentAnimState], unitSprite);
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
	auto events = unitAnimPlayer.update(dt, unitSprite);
	
	if (any(events & AnimationEvent::FINAL_FRAME) || unitAnimPlayer.currentFrame == 0)
		unitHitboxes.clear();
	else if (any(events & AnimationEvent::ATTACK))
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
	unitGear = std::clamp((unitGear + 1), 1, highestGear);
	setup_workshop_unit(unitId, unitGear);
}

// BUTTON INDEXEX /////////////////////////////////////////////////////
#pragma region Button Indexs
Button& WorkshopMenu::return_btn() { return buttonManager.buttons[static_cast<int>(ButtonIndex::RETURN)]; }
Button& WorkshopMenu::pause_btn() { return buttonManager.buttons[static_cast<int>(ButtonIndex::PAUSE)]; }
Button& WorkshopMenu::switchGearBtn() { return buttonManager.buttons[static_cast<int>(ButtonIndex::SWITCH_GEAR)]; }
Button& WorkshopMenu::animationSpeedBtn() { return buttonManager.buttons[static_cast<int>(ButtonIndex::SPEED_UP)]; }

Button& WorkshopMenu::animation_btn(UnitAnimationState ani) {
	auto i = static_cast<int>(ani);
	if (i < 0 || i > 4) i = 5; // if its a special animation
	return animation_btn(i);
}
Button& WorkshopMenu::animation_btn(int i) { return buttonManager.buttons[i + static_cast<int>(ButtonIndex::ANIMATIONS)]; }

int WorkshopMenu::stat_index(const std::string& str) const {
	switch (str[0]) {
	case 'a': return 0; // attack_time
	case 'c': return 1; // cost
	case 'h': return 2 + str[1] != 'i'; // hits(2) / hp(3)
	case 'k': return 4; // knockbacks
	case 'r': return 5; // recharge_time
	case 's': return 6 + str[1] != 'i'; // sight_range(7) / speed(8)
	default:
		std::cout << "could not find stat index: " << str[0] << std::endl;
		return 0;
	}

	return 0;
}
#pragma endregion

#pragma region Static Functions
std::array<sf::Sprite, 8> WorkshopMenu::make_statIcons() {
	return {
		sf::Sprite(t_workshopStatsIcons), sf::Sprite(t_workshopStatsIcons),
		sf::Sprite(t_workshopStatsIcons), sf::Sprite(t_workshopStatsIcons),
		sf::Sprite(t_workshopStatsIcons), sf::Sprite(t_workshopStatsIcons),
		sf::Sprite(t_workshopStatsIcons), sf::Sprite(t_workshopStatsIcons),
	};
}
std::array<sf::Text, 8> WorkshopMenu::make_statTexts() {
	std::array<sf::Text, 8> icons = {
	sf::Text(baseFont),sf::Text(baseFont),sf::Text(baseFont),
	sf::Text(baseFont),sf::Text(baseFont),sf::Text(baseFont),
	sf::Text(baseFont),sf::Text(baseFont)
	};

	return icons;
}
#pragma endregion