#include "pch.h"
#include "WorkshopMenu.h"
#include "UnitConfig.h"
#include "Camera.h"
#include "UILayout.h"
#include "UITextures.h"
#include "UnitSaveData.h"
#include "PlayerSaveData.h"
#include "Utils.h"

using namespace Textures::UI;
using namespace UI::Workshop;
using namespace UI::Colors;

WorkshopMenu::WorkshopMenu(Camera& cam) :
	Menu(cam), 
	statIcons(make_statIcons()),
	statTexts(make_statTexts())
{
	Screen::setFontSize(unitNameText, UNIT_TEXT_SIZE);

	Screen::setFontSize(statTexts[0], STAT_TEXT_SIZE);
	unsigned int fontSize = statTexts[0].getCharacterSize();
	for (int i = 0; i < STAT_ICONS; i++) {
		statTexts[i].setCharacterSize(fontSize);
		statIcons[i].setTextureRect(r_workshopStatsIcons[i]);
		statIcons[i].setOrigin(statIcons[i].getLocalBounds().size * 0.5f);
	}
	 
	pause_btn().setup(UNIT_PAUSE_BTN_POS, UNIT_PAUSE_BTN_SIZE, t_pauseBtn);
	return_btn().setup(RETURN_BTN_POS, RETURN_BTN_SIZE, t_returnBtn);
	switchGearBtn().setup(UNIT_SWITCH_GEAR_BTN_POS, UNIT_SWITCH_GEAR_BTN_SIZE, t_switchGearBtn);
	animationSpeedBtn().setup(UNIT_SPEED_BTN_POS, UNIT_SPEED_BTN_SIZE, t_speedUpBtn);
	upgradeUnitBtn().setup(UPGRADE_UNIT_BTN_POS, UPGRADE_UNIT_BTN_SIZE, t_speedUpBtn);

	pause_btn().onClick = [this](bool isM1) {if (isM1) paused = !paused; };
	switchGearBtn().onClick = [this](bool isM1) {if (isM1) switch_unit_gear(); };
	animationSpeedBtn().onClick = [this](bool isM1) {if (isM1) unitAnimSpeedIndex = (unitAnimSpeedIndex + 1) % 5; };
	upgradeUnitBtn().onClick = [this](bool isM1) {if (isM1) upgrade_unit(); };

	for (int i = 0; i < UnitConfig::TOTAL_ANIM_COUNT; i++) {
		// Setting the positions for animPlayer buttons takes a lot fo lines, so its done in reset_positions()
		animation_btn(i).setup({}, UNIT_ANIMATION_BTN_SIZE, t_workshopAnimBtns, r_workshopAnimBtns[i]);
		animation_btn(i).onClick = [i, this](bool isM1) {
			if (!isM1) return;
			auto newAnimState = static_cast<UnitAnimationState>(i);
			unitHitboxes.clear();

			if (newAnimState != currentAnimState)
				unitAnimPlayer.start(&unitAnimMap[newAnimState], unitSprite);
			else unitAnimPlayer.reset(unitSprite);

			currentAnimState = newAnimState;
			};
	}
}

void WorkshopMenu::reset_positions() {
	return_btn().set_pos(Screen::toPixels(RETURN_BTN_POS));
	pause_btn().set_pos(Screen::toPixels(UNIT_PAUSE_BTN_POS));
	unitNameText.setPosition(Screen::toPixels(UNIT_NAME_TEXT_POS));
	unitDescText.setPosition(Screen::toPixels(UNIT_DESC_TEXT_POS));
	unitSprite.setPosition(Screen::toPixels(UNIT_POS));

	sf::Vector2f iconPos = Screen::toPixels(STAT_ICON_POS);
	sf::Vector2f spacing = Screen::toPixels(STAT_ICON_SPACING);
	sf::Vector2f offset = Screen::toPixels(STAT_TEXT_OFFSET);

	for (int i = 0; i < STAT_ICONS; i++) {
		statIcons[i].setPosition(iconPos);
		statTexts[i].setPosition(iconPos + offset);
		iconPos += spacing;
	}

	set_unit_anim_btn_positions();
}

#pragma region Set Up UI For A New Unit
void WorkshopMenu::setup_workshop_unit(int id, int gear, float enemyMagnification) {
	unitId = id;
	unitGear = gear;
	highestGear = UnitConfig::getMaxGear(id);

	const nlohmann::json unitJson = UnitConfig::createUnitJson(id, gear);

	unitHitboxes.clear();
	unitAnimTextures.clear();
	unitAnimMap.clear();

	unitStats = id < 100 ? UnitStats::create_player(unitJson) : UnitStats::create_enemy(unitJson, enemyMagnification);
	AnimationClip::setup_unit_animation_map(unitJson, unitAnimMap, unitAnimTextures);

	// Create the Unit's Animations and stat texts
	for (auto& [state, animPlayer] : unitAnimMap) animPlayer.loops = true;
	currentAnimState = UnitAnimationState::MOVE;
	unitAnimPlayer.start(&unitAnimMap[currentAnimState], unitSprite);
	set_stat_texts(unitJson);

	// Set certain buttons invisible if viewing an enemy
	set_team_specific_ui_visibility();	
}
void WorkshopMenu::set_team_specific_ui_visibility() {
	upgradeUnitBtn().visible = !isViewingAnEnemy();
	
	// Set unavailable animations invisible
	// First turns them all invisible, then the actually used ones visible
	for (int i = 0; i < UnitConfig::TOTAL_ANIM_COUNT; i++)
		animation_btn(i).visible = false;
	for (const auto& [animState, anim] : unitAnimMap) 
		animation_btn(animState).visible = true;

	if (!isViewingAnEnemy()) {
		int cost = UnitConfig::GetUpgradeCost(UnitSaveData::GetLevel(unitId));
		int curScrapParts = PlayerSaveData::GetScrapParts();

		scrapPartsText.setString(std::format("{}", curScrapParts));
		upgradeCostText.setString(std::format("{}", cost));

		sf::Color upgradeUIColor = (curScrapParts < cost) ? UI::Colors::GREY : sf::Color::White;
		upgradeUnitBtn().sprite.setColor(upgradeUIColor);
		upgradeCostText.setFillColor(upgradeUIColor);
	}
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
void WorkshopMenu::set_unit_anim_btn_positions() {
	sf::Vector2f animPos = UNIT_ANIMATION_BTN_POS;

	for (int i = 0; i < 6; i++) {
		animation_btn(i).set_pos(Screen::toPixels(animPos));
		animPos += UNIT_ANIMATION_BTN_INCREMENT;
	}

	if (unitId == -1) return;

	for (const auto& [animState, anim] : unitAnimMap) {
		if (isDefaultUnitAnim(animState)) continue;
		animation_btn(animState).set_pos(Screen::toPixels(animPos));
		animPos += UNIT_ANIMATION_BTN_INCREMENT;
	}
}
#pragma endregion

void WorkshopMenu::draw() {
	buttonManager.draw(cam);


	// Drawing non-button Sprites and texts
	for (int i = 0; i < STAT_ICONS; i++) {
		cam.renderer.queue_ui_draw(&statIcons[i]);
		cam.renderer.queue_ui_draw(&statTexts[i]);
	}

	cam.renderer.queue_world_draw(&unitSprite);
	cam.renderer.queue_ui_draw(&unitNameText);
	cam.renderer.queue_ui_draw(&unitDescText);

	if (!isViewingAnEnemy()) {
		cam.renderer.queue_ui_draw(&scrapPartsIcon);
		cam.renderer.queue_ui_draw(&scrapPartsText);
		cam.renderer.queue_ui_draw(&upgradeCostText);
	}

	draw_unit_hurtbox();
	draw_unit_hitboxs();
}
void WorkshopMenu::draw_unit_hurtbox() {
	sf::Vector2f size = Screen::toPixels(UNIT_ANIMATION_BTN_SIZE);
	sf::RectangleShape hurtbox(size);
	sf::Vector2f origin = { (!unitStats.is_player() ? 0.f : size.x) , size.y };

	hurtbox.setOrigin(origin);
	hurtbox.setPosition(unitSprite.getPosition());
	hurtbox.setFillColor(sf::Color(3, 252, 198, 128));

	cam.renderer.queue_temp_ui_draw(hurtbox);
}
void WorkshopMenu::draw_unit_hitboxs() {
	for (auto& hitbox : unitHitboxes)
		cam.renderer.queue_world_draw(&hitbox);
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
void WorkshopMenu::upgrade_unit() {
	if (isViewingAnEnemy()) return;

	int cost = UnitConfig::GetUpgradeCost(UnitSaveData::GetLevel(unitId));

	if (auto curScrapParts = PlayerSaveData::SpendScrapParts(cost)) {
		int newLevel = UnitSaveData::IncreaseLevel(unitId);
		int newCost = UnitConfig::GetUpgradeCost(newLevel);

		upgradeCostText.setString(std::format("{}", newCost));
		scrapPartsText.setString(std::format("{}", curScrapParts.value()));

		sf::Color upgradeUIColor = (curScrapParts < newCost) ? UI::Colors::GREY : sf::Color::White;
		upgradeUnitBtn().sprite.setColor(upgradeUIColor);
		upgradeCostText.setFillColor(upgradeUIColor);
	}
}

// BUTTON INDEXEX /////////////////////////////////////////////////////
#pragma region Button Indexs
Button& WorkshopMenu::return_btn() { return buttonManager.buttons[static_cast<int>(ButtonIndex::RETURN)]; }
Button& WorkshopMenu::pause_btn() { return buttonManager.buttons[static_cast<int>(ButtonIndex::PAUSE)]; }
Button& WorkshopMenu::switchGearBtn() { return buttonManager.buttons[static_cast<int>(ButtonIndex::SWITCH_GEAR)]; }
Button& WorkshopMenu::animationSpeedBtn() { return buttonManager.buttons[static_cast<int>(ButtonIndex::SPEED_UP)]; }
Button& WorkshopMenu::upgradeUnitBtn() { return buttonManager.buttons[static_cast<int>(ButtonIndex::UPGRADE_UNIT)]; }

Button& WorkshopMenu::animation_btn(UnitAnimationState ani) {
	switch (ani) {
	case UnitAnimationState::PHASE_WINDDOWN: return animation_btn(9);
	case UnitAnimationState::PHASE_WINDUP: return animation_btn(10);
	case UnitAnimationState::WAITING_TO_DELETE: 
		throw std::runtime_error("Workshop cannot display the \"Waiting to Delete\" animation.");
		return animation_btn(0);
	default: return animation_btn(static_cast<int>(ani));
	}
}
Button& WorkshopMenu::animation_btn(int i) { return buttonManager.buttons[i + static_cast<int>(ButtonIndex::FIRST_ANIMATION)]; }

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