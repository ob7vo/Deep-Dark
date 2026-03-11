#include "pch.h"
#include "StagePreviewMenu.h"
#include "Camera.h"
#include "UITextures.h"
#include "Utils.h"

using namespace Textures::UI;
using namespace UI::ArmoryMenu::StagePreview;

StagePreviewMenu::StagePreviewMenu(Camera& cam) : Menu(cam) {
	backgroundSprite.setTexture(t_menuBG1);
	backgroundSprite.setScale(Screen::getSpriteScale(backgroundSprite, BACKGROUND_SIZE));

	startStageSetText.setString("beebee next stage");
	Screen::setFontSize(startStageSetText, TEXT_HEIGHT);

	for (int i = 0; i < 10; i++) {
		sf::Vector2f scale = Screen::getSpriteScale(usedUnitsSlotSprites[i], UNIT_SLOT_SCALE);
		usedUnitsSlotSprites[i].setScale(scale);
	}

	startStageBtn().setup(START_BTN_POS, START_BTN_SIZE, t_startBtn);
	exitStageBtn().setup(EXIT_BTN_POS, ENEMY_UNITS_SIZE, t_returnBtn);
	closeBtn().setup(CLOSE_MENU_BTN_POS, CLOSE_MENU_BTN_SIZE, t_closeBtn);
}
void StagePreviewMenu::setup_menu(int stage, int set, const std::array<std::pair<int, int>, 10>& newUsedUnits) {
	reset_sprites();

	stageId = stage;
	stageSet = set;

	std::ifstream file(std::format("configs/stage_data/stage_{}.json", stageId));
	const nlohmann::json stageSetJson = nlohmann::json::parse(file)["sets"][set];

	create_enemy_sprites(stageSetJson);

	// The positions of the Slot Sprites are already set in reset_positions()
	// Just set the Textures to the slots of the Units used in the last Stage Set (which if none, if entering from StageSelectMenu)
	for (int i = 0; i < 10; i++) {
		const auto [id, gear] = newUsedUnits[i];

		usedUnitsSlotSprites[i].setTexture(getUnitSlot(id, gear), true);
		if (id != -1) usedUnits[id] = true;
	}

	// Will reset restrictions that check if there are any to add
	RestrictionParser::apply_unit_restrictions(unitRestrictions, stageSetJson);
}
void StagePreviewMenu::create_enemy_sprites(const nlohmann::json& stageSetJson) {
	// There can be multiple spawners for the same enemy type
	// SO keep track of the ones include to not duplicate sprites

	std::bitset<UnitConfig::TOTAL_ENEMY_UNITS> includedEnemies;

	// Get the Number of Unique Enemies
	std::unordered_set<int> uniqueEnemyIds = {};
	for (const auto& enemy : stageSetJson["enemy_spawns"]) 
		uniqueEnemyIds.insert(enemy["unit_id"].get<int>());

	// Reserve the vectors to fix dangling Texture pointers
	size_t uniqueEnemyCount = uniqueEnemyIds.size();
	enemyUnitTextures.reserve(uniqueEnemyCount);
	enemyUnitSprites.reserve(uniqueEnemyCount);

	// Creating the enemy Sprits
	for (const auto& enemy : stageSetJson["enemy_spawns"]) {
		int id = enemy["unit_id"].get<int>();
		int gear = enemy.value("unit_gear", 1);

		if (includedEnemies[id - UnitConfig::ENEMY_ID_OFFSET]) continue;
		includedEnemies[id - UnitConfig::ENEMY_ID_OFFSET] = true;

		sf::Texture& unitTexture = enemyUnitTextures.emplace_back();

		// Get and use the json for the Enemies Idle Sprite Sheet
		// The Sprites will be of the enemy's first Idle  frame 
		const nlohmann::json idleAnimConfig = UnitConfig::createUnitJson(id, gear)["animations"]["idle"];
		std::string idleAnimPath = UnitConfig::getUnitGearPath(id, gear) + "idle.png";
		sf::Vector2i cellSize = { idleAnimConfig["cell_size"][0], idleAnimConfig["cell_size"][1] };

		Textures::loadTexture(unitTexture, idleAnimPath);

		sf::Sprite& enemySprite = enemyUnitSprites.emplace_back(unitTexture);
		enemySprite.setTextureRect({ {0, 0}, cellSize });
		enemySprite.setOrigin({ (float)unitTexture.getSize().x * .5f, (float)unitTexture.getSize().y });
		enemySprite.setScale(Screen::getSpriteScale(enemySprite, ENEMY_UNITS_SIZE));
	}

	// Moving the enemy sprites into place
	sf::Vector2f center = Screen::toPixels(ENEMY_UNITS_CENTER);
	sf::Vector2f spacing = Screen::toPixels(ENEMY_UNITS_SPACING); // y == 0
	sf::Vector2f left = { center.x - (spacing.x * ((float)uniqueEnemyCount - 1.f) * 0.5f), center.y};

	for (size_t i = 0; i < uniqueEnemyCount; i++)
		enemyUnitSprites[i].setPosition(left + (spacing * (float)i));
}

void StagePreviewMenu::reset_positions() {
	startStageBtn().set_pos(Screen::toPixels(START_BTN_POS));
	exitStageBtn().set_pos(Screen::toPixels(EXIT_BTN_POS));
	closeBtn().set_pos(Screen::toPixels(CLOSE_MENU_BTN_POS));

	sf::Vector2f slotPos = Screen::toPixels(FIRST_UNIT_SLOT_POS);
	sf::Vector2f slotInc = Screen::toPixels(UNIT_SLOT_INCREMENT);
	float startX = slotPos.x;

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 5; j++) {
			int k = i * 5 + j;

			usedUnitsSlotSprites[k].setPosition(slotPos);
			slotPos.x += slotInc.x;
		}

		slotPos.x = startX;
		slotPos.y += slotInc.y;
	}

	startStageSetText.setPosition(Screen::toPixels(TEXT_POS));
	backgroundSprite.setPosition(Screen::toPixels(BACKGROUND_POS));
}
void StagePreviewMenu::reset_sprites() {
	enemyUnitTextures = {};
	enemyUnitSprites = {};

	for (int i = 0; i < 10; i++) usedUnitsSlotSprites[i].setTexture(t_defaultUnitSlot);
}
void StagePreviewMenu::full_reset() {
	stageId = 0;
	stageSet = 0;

	usedUnits.reset();
	unitRestrictions.reset();

	reset_sprites();
}

void StagePreviewMenu::draw() {
	cam.renderer.darken_screen();
	buttonManager.draw(cam);

	cam.renderer.queue_ui_draw(&backgroundSprite);
	cam.renderer.queue_ui_draw(&startStageSetText);

	// Drawing Units
	for (int i = 0; i < 10; i++) cam.renderer.queue_ui_draw(&usedUnitsSlotSprites[i]);
	for (auto& enemy : enemyUnitSprites) cam.renderer.queue_ui_draw(&enemy);
}


// Static Methods
std::array<sf::Sprite, 10> StagePreviewMenu::make_unitSlotSprites() {
	const sf::Texture& tex = t_defaultUnitSlot;
	return {
		sf::Sprite(tex), sf::Sprite(tex),sf::Sprite(tex),sf::Sprite(tex),
		sf::Sprite(tex),sf::Sprite(tex),sf::Sprite(tex),sf::Sprite(tex),
		sf::Sprite(tex),sf::Sprite(tex)
	};
}