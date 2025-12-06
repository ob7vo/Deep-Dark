#include "pch.h"
#include "StageSetMenu.h"
#include "Camera.h"
#include "Animation.h"

using namespace UI::StageSet;

StageSetMenu::StageSetMenu(Camera& cam) : Menu(cam) {
	backgroundSprite.setTexture(TextureManager::t_menuBG1);
	backgroundSprite.setScale(cam.get_norm_sprite_scale(backgroundSprite, BACKGROUND_SIZE));

	startStageSetText.setString("beebee next stage");
	startStageSetText.setCharacterSize(cam.get_norm_font_size(startStageSetText, TEXT_HEIGHT));

	for (int i = 0; i < 10; i++) {
		sf::Vector2f scale = cam.get_norm_sprite_scale(usedUnitsSlotSprites[i], UNIT_SLOT_SCALE);
		usedUnitsSlotSprites[i].setScale(scale);
	}

	startStageBtn().setup(START_BTN_POS, START_BTN_SIZE, cam, TextureManager::t_startBtn);
	exitStageBtn().setup(EXIT_BTN_POS, ENEMY_UNITS_SIZE, cam, TextureManager::t_returnBtn);
	closeBtn().setup(CLOSE_MENU_BTN_POS, CLOSE_MENU_BTN_SIZE, cam, TextureManager::t_closeBtn);
}
void StageSetMenu::setup_menu(int stage, int set, const std::array<std::pair<int, int>, 10>& units) {
	reset_sprites();

	stageId = stage;
	stageSet = set;

	std::ifstream file(std::format("configs/stage_data/stage_{}.json", stageId));
	const nlohmann::json stageSetJson = nlohmann::json::parse(file)["sets"][set];

	create_enemy_sprites(stageSetJson);

	// The positions of the Slot Sprites are already set in reset_positions()
	// Just set the Textures to the slots of the Units used in the last Stage Set (which if none, if entering from StageSelectMenu)
	for (int i = 0; i < 10; i++) {
		const auto [id, gear] = units[i];
		usedUnitsSlotSprites[i].setTexture(TextureManager::getUnitSlot(id, gear), true);
	}
}
void StageSetMenu::create_enemy_sprites(const nlohmann::json& stageSetJson) {
	// There can be multiple spawners for the same enemy type
	// SO keep track of the ones include to not duplicate sprites

	std::bitset<UnitData::TOTAL_ENEMY_UNITS> includedEnemies;

	// Get the Number of Unique Enemies
	std::unordered_set<int> uniqueEnemyIds = {};
	for (const auto& enemy : stageSetJson["enemy_spawns"]) 
		uniqueEnemyIds.insert(enemy["unit_id"].get<int>());

	// Reserve the vectors to fix dangling Texture pointers
	size_t uniqueEnemyCount = uniqueEnemyIds.size();
	enemyUnitTextures.reserve(uniqueEnemyCount);
	enemyUnitSprites.reserve(uniqueEnemyCount);

	for (const auto& enemy : stageSetJson["enemy_spawns"]) {
		int id = enemy["unit_id"].get<int>();

		// Enmemy ids start from 100, so subtract 100 to properly index them
		if (includedEnemies[id - 100]) continue;
		includedEnemies[id - 100] = true;

		sf::Texture& unitTexture = enemyUnitTextures.emplace_back();

		// Get and use the json for the Enemys Idle Sprite Sheet
		// The Sprites will be of the enemy's first Idle  frame 
		const nlohmann::json idleAnimConfig = UnitData::createUnitJson(id)["animations"]["idle"];
		std::string idleAnimPath = UnitData::get_unit_folder_path(id) + "idle.png";
		sf::Vector2i cellSize = { idleAnimConfig["cell_size"][0], idleAnimConfig["cell_size"][1] };

		if (!unitTexture.loadFromFile(idleAnimPath)) std::cout << "FUCK: " << id << std::endl;

		sf::Sprite& enemySprite = enemyUnitSprites.emplace_back(unitTexture);
		enemySprite.setTextureRect({ {0, 0}, cellSize });
		enemySprite.setOrigin({ (float)unitTexture.getSize().x * .5f, (float)unitTexture.getSize().y });
		enemySprite.setScale(cam.get_norm_sprite_scale(enemySprite, ENEMY_UNITS_SIZE));
	}

	sf::Vector2f center = cam.norm_to_pixels(ENEMY_UNITS_CENTER);
	sf::Vector2f spacing = cam.norm_to_pixels(ENEMY_UNITS_SPACING); // y == 0
	sf::Vector2f left = { center.x - (spacing.x * ((float)uniqueEnemyCount - 1.f) * 0.5f), center.y};

	for (size_t i = 0; i < uniqueEnemyCount; i++)
		enemyUnitSprites[i].setPosition(left + (spacing * (float)i));
}

void StageSetMenu::reset_positions() {
	startStageBtn().set_norm_pos(START_BTN_POS, cam);
	exitStageBtn().set_norm_pos(EXIT_BTN_POS, cam);
	closeBtn().set_norm_pos(CLOSE_MENU_BTN_POS, cam);

	sf::Vector2f slotPos = cam.norm_to_pixels(FIRST_UNIT_SLOT_POS);
	sf::Vector2f slotInc = cam.norm_to_pixels(UNIT_SLOT_INCREMENT);
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

	startStageSetText.setPosition(cam.norm_to_pixels(TEXT_POS));
	backgroundSprite.setPosition(cam.norm_to_pixels(BACKGROUND_POS));
}
void StageSetMenu::reset_sprites() {
	enemyUnitTextures = {};
	enemyUnitSprites = {};

	for (int i = 0; i < 10; i++) usedUnitsSlotSprites[i].setTexture(TextureManager::t_defaultUnitSlot);
}
void StageSetMenu::full_reset() {
	stageId = 0;
	stageSet = 0;

	usedUnits.reset();
	unitViolatesCondition.reset();

	reset_sprites();
}

void StageSetMenu::draw() {
	cam.darken_screen();
	buttonManager.draw(cam);

	cam.queue_ui_draw(&backgroundSprite);
	cam.queue_ui_draw(&startStageSetText);

	// Drawing Units
	for (int i = 0; i < 10; i++) cam.queue_ui_draw(&usedUnitsSlotSprites[i]);
	for (auto& enemy : enemyUnitSprites) cam.queue_ui_draw(&enemy);
}