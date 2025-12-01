#include "pch.h"
#include "StageSetMenu.h"
#include "Camera.h"

using namespace UI::StageSet;

StageSetMenu::StageSetMenu(Camera& cam) : Menu(cam) {
	backgroundSprite.setTexture(TextureManager::t_menuBG1);
	backgroundSprite.setScale(cam.get_norm_sprite_scale(backgroundSprite, BACKGROUND_SIZE));
	startStageSetText.setCharacterSize(cam.get_norm_font_size(startStageSetText, TEXT_HEIGHT));

	for (int i = 0; i < 10; i++) {
		sf::Vector2f scale = cam.get_norm_sprite_scale(usedUnitsSlotSprites[i], UNIT_SLOT_SCALE);
		usedUnitsSlotSprites[i].setScale(scale);
	}

//	startStageBtn().setup()
}

void StageSetMenu::reset_positions() {

}
void StageSetMenu::setup_menu(const std::vector<int>& ids, int stage, int set) {
	stageId = stage;
	stageSet = set; 

	std::ifstream file(std::format("configs/stage_data/stage_{}.json"), stageId);
	const nlohmann::json stageJson = nlohmann::json::parse(file)["sets"][set];

	//for (const auto& enemy : stageJson["enemy_spawns"])
}

