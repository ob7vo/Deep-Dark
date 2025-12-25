#pragma once
#include "TextureManager.h"
#include "UnitDataConstants.h"

namespace Textures::UI {
	// General Buttons
	extern sf::Texture t_returnBtn;
	extern sf::Texture t_closeBtn;
	extern sf::Texture t_pauseBtn;
	extern sf::Texture t_startBtn;
	extern sf::Texture t_settingsBtn;
	extern sf::Texture t_slider;

	// Stage Select
	extern sf::Texture t_stageNodeBtn;
	extern sf::Texture t_armoryBtn;
	// Stage UI
	extern sf::Texture t_upgradeWalletBtn;
	extern sf::Texture t_fireCannonBtn;
	// Start Menu
	extern sf::Texture t_quitBtn;
	extern sf::Texture t_startGameBtn;
	// WorkShop
	extern sf::Texture t_switchGearBtn;
	extern sf::Texture t_speedUpBtn;
	// Armory Menu
	extern sf::Texture t_enterStageSetBtn;

	extern sf::Texture t_menuBG1;
	extern sf::Texture t_menuBG2;

	extern sf::Texture t_workshopAnimBtns;
	extern std::array<sf::IntRect, 7> r_workshopAnimBtns;
	extern sf::Texture t_workshopStatsIcons;
	extern std::array<sf::IntRect, 8> r_workshopStatsIcons;

	extern sf::Texture t_defaultUnitSlot;
	extern std::array<std::array<sf::Texture, 3>, UnitData::TOTAL_PLAYER_UNITS> t_unitSlots;

	const sf::Texture& getUnitSlot(int id, int gear);
	const sf::Texture& getUnitSlot(std::pair<int, int> unit);

	void initialize();
}