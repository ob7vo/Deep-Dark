#include "pch.h"
#include "UITextures.h"
#include "Utils.h"
#include "UnitData.h"

using namespace FolderPaths;

namespace Textures::UI {
	// General Buttons
	sf::Texture t_returnBtn = createTexture(path(uiPath, "return_btn.png"));
	sf::Texture t_pauseBtn = createTexture(path(uiPath, "pause_btn.png"));
	sf::Texture t_startBtn = createTexture(path(uiPath, "start_btn.png"));
	sf::Texture t_closeBtn = createTexture(path(uiPath, "close_btn.png"));
	sf::Texture t_settingsBtn = createTexture(path(uiPath, "settings_btn.png"));
	sf::Texture t_slider = createTexture(path(uiPath, "slider_btn.png"));

	// Stage Select
	sf::Texture t_stageNodeBtn = createTexture(path(stageSelectPath, "stage_node.png"));
	sf::Texture t_armoryBtn = createTexture(path(stageSelectPath, "armory_btn.png"));
	// Stage UI
	sf::Texture t_upgradeWalletBtn = createTexture(path(stageUIPath, "upgrade_bag.png"));
	sf::Texture t_fireCannonBtn = createTexture(path(stageUIPath, "fire_cannon.png"));
	// Start Menu
	sf::Texture t_quitBtn = createTexture(path(startMenuPath, "quit_btn.png"));
	sf::Texture t_startGameBtn = createTexture(path(startMenuPath, "start_btn.png"));
	// WorkShop
	sf::Texture t_switchGearBtn = createTexture(path(workshopPath, "switch_gear.png"));
	sf::Texture t_speedUpBtn = createTexture(path(workshopPath, "speed_up.png"));
	// Armory Menu
	sf::Texture t_enterStageSetBtn = createTexture(path(armoryPath, "enter_stage_set_btn.png"));

	// MENUS  ----------------------------------------------------------------------------
	sf::Texture t_menuBG1 = createTexture(path(uiPath, "menu1.png"));
	sf::Texture t_menuBG2 = createTexture(path(uiPath, "menu2.png"));
	// MENUS -----------------------------------------------------------------------------

	sf::Texture t_workshopAnimBtns = createTexture(path(workshopPath, "unit_animation_btns.png"));
	std::array<sf::IntRect, 7> r_workshopAnimBtns;
	sf::Texture t_workshopStatsIcons = createTexture(path(workshopPath, "unit_stats_icons.png"));
	std::array<sf::IntRect, 8> r_workshopStatsIcons;

	sf::Texture t_defaultUnitSlot = createTexture("sprites/defaults/empty_slot.png");
	std::array<std::array<sf::Texture, 3>, UnitData::TOTAL_PLAYER_UNITS> t_unitSlots;

	const sf::Texture& getUnitSlot(int id, int gear) {
			if (gear < 1 || gear > 3) {
				throw InvalidGearError(gear, id);
				return t_defaultUnitSlot;
			}
			else if (id <= -1) return t_defaultUnitSlot;


			return t_unitSlots[id][gear - 1];
	}
	const sf::Texture& getUnitSlot(std::pair<int, int> unit) { return getUnitSlot(unit.first, unit.second); }

	void initialize() {
		r_workshopAnimBtns = createTextureRects<7>(t_workshopAnimBtns.getSize(), { 32,32 });
		r_workshopStatsIcons = createTextureRects<8>(t_workshopStatsIcons.getSize(), { 32,32 });

		for (int id = 0; id < UnitData::TOTAL_PLAYER_UNITS; id++)
			for (int gear = 1; gear <= UnitData::getMaxGear(id); gear++)
				t_unitSlots[id][gear - 1] = UnitData::createSlotTexture(id, gear);
	}
}
