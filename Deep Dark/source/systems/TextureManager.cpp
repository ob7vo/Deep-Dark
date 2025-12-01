#include "pch.h"
#include "TextureManager.h"
#include "UnitData.h"
#include "Utils.h"

using namespace FolderPaths;

static sf::Texture createTexture(const std::string& path) {
	sf::Texture tex;
	TextureManager::loadTexture(tex, path);
	return tex;
}

sf::Texture TextureManager::t_statusIcons = createTexture(path(effectsPath, "status_icons.png"));
std::array<sf::IntRect, 8> TextureManager::r_statusIcons;

sf::Texture TextureManager::t_workshopAnimBtns = createTexture(path(workshopPath, "unit_animation_btns.png"));
std::array<sf::IntRect, 7> TextureManager::r_workshopAnimBtns;
sf::Texture TextureManager::t_workshopStatsIcons = createTexture(path(workshopPath, "unit_stats_icons.png"));
std::array<sf::IntRect, 8> TextureManager::r_workshopStatsIcons;

std::array<std::array<sf::Texture, 3>, 3> TextureManager::t_unitSlots;

sf::Texture TextureManager::t_defaultUnitSlot = createTexture("sprites/defaults/empty_slot.png");

// START OF BUTTONS REGION -----------------------------------------------------------------
#pragma region Buttons
sf::Texture TextureManager::t_returnBtn = createTexture(path(uiPath, "return_btn.png"));
sf::Texture TextureManager::t_pauseBtn = createTexture(path(uiPath, "pause_btn.png"));
sf::Texture TextureManager::t_closeBtn = createTexture(path(uiPath, "close_btn.png"));
sf::Texture TextureManager::t_settingsBtn = createTexture(path(uiPath, "settings_btn.png"));
sf::Texture TextureManager::t_slider = createTexture(path(uiPath, "slider_btn.png"));

// Stage Select
sf::Texture TextureManager::t_stageNodeBtn = createTexture(path(stageSelectPath, "stage_node.png"));
sf::Texture TextureManager::t_armoryBtn = createTexture(path(stageSelectPath, "armory_btn.png"));
// Stage UI
sf::Texture TextureManager::t_upgradeWalletBtn = createTexture(path(stageUIPath, "upgrade_bag.png"));
sf::Texture TextureManager::t_fireCannonBtn = createTexture(path(stageUIPath, "fire_cannon.png"));
// Start Menu
sf::Texture TextureManager::t_quitBtn = createTexture(path(startMenuPath, "quit_btn.png"));
sf::Texture TextureManager::t_startGameBtn = createTexture(path(startMenuPath, "start_btn.png"));
// WorkShop
sf::Texture TextureManager::t_switchGearBtn = createTexture(path(workshopPath, "switch_gear.png"));
sf::Texture TextureManager::t_speedUpBtn = createTexture(path(workshopPath, "speed_up.png"));
#pragma endregion
// END OF BUTTONS REGION --------------------------------------------------------------------

// MENUS REGION ----------------------------------------------------------------------------
sf::Texture TextureManager::t_menuBG1 = createTexture(path(uiPath, "menu1.png"));
sf::Texture TextureManager::t_menuBG2 = createTexture(path(uiPath, "menu2.png"));


void TextureManager::initialize() {
	r_statusIcons = createTextureRects<8>(t_statusIcons.getSize(), { 32,23 });
	r_workshopAnimBtns = createTextureRects<7>(t_workshopAnimBtns.getSize(), { 32,32 });
	r_workshopStatsIcons = createTextureRects<8>(t_workshopStatsIcons.getSize(), { 32,32 });

	for (int id = 0; id < UnitData::TOTAL_PLAYER_UNITS; id++)
		for (int gear = 1; gear <= 3; gear++)
			t_unitSlots[id][gear - 1] = UnitData::createSlotTexture(id, gear, false);
}

void TextureManager::loadTexture(sf::Texture& tex, const std::string& path, bool printErrorMsg) {
	if (!tex.loadFromFile(path)) {
		if (printErrorMsg) std::cout << "Could not load a texture from the path: [" << path << "]" << std::endl;
		tex = defTex;
	}
}

void TextureManager::fillTextureRects(std::span<sf::IntRect> spanRects, sf::Vector2u texSize, sf::Vector2i cellSize) {
	if (cellSize.x <= 0 || cellSize.y <= 0) {
		std::cerr << "Invalid cell size: One or mreo Component is less than 0" << std::endl;
	}
	
	size_t FRAMES = spanRects.size();
	int columns = texSize.x / cellSize.x;
	int rows = texSize.y / cellSize.y;
	int totalCells = columns * rows;

	if (texSize.x % cellSize.x != 0 || texSize.y % cellSize.y != 0)
		std::cout << "Cell sizes did not line up with the Texture" << std::endl;
	if (FRAMES > totalCells) {
		std::cerr << "Warning: frameCount (" << FRAMES
			<< ") exceeds available cells (" << totalCells << ")" << std::endl;
	}

	int frame = 0;
	for (int row = 0; row < rows && frame < FRAMES; row++) {
		for (int col = 0; col < columns && frame < FRAMES; col++) {
			sf::IntRect rect({ col * cellSize.x, row * cellSize.y }, cellSize);
			spanRects[frame++] = rect;
		}
	}

	if (frame != FRAMES)
		std::cerr << "Warning: Only loaded " << frame << " of " << FRAMES << " frames" << std::endl;
}
std::vector<sf::IntRect> TextureManager::createTextureRects(int frames, sf::Vector2u texSize, sf::Vector2i cellSizes) {
	std::vector<sf::IntRect> rects(frames);
	fillTextureRects(std::span(rects), texSize, cellSizes);
	return rects;
}

sf::Texture& TextureManager::getUnitSlot(int id, int gear) {
	if (gear < 1 || gear > 3) {
		std::cout << "Gear must be between 1 and 3: The gear was: # " << gear << std::endl;
		return t_defaultUnitSlot;
	}
	else if (id <= -1) return t_defaultUnitSlot;


	return t_unitSlots[id][gear - 1];
}
sf::Texture& TextureManager::getUnitSlot(std::pair<int, int> unit) { return getUnitSlot(unit.first, unit.second); }