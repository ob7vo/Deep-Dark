#pragma once
#include <SFML/Graphics.hpp>
#include <span>

const sf::Texture defTex("sprites/defaults/defaultTexture.png");

struct TextureManager {
	static const sf::Texture dTex;
	static sf::Texture t_defaultUnitSlot;

	// START OF BUTTONS REGION --------------------------------------------
	#pragma region Buttons
	// Common Buttons (reused a lot)
	static sf::Texture t_returnBtn;
	static sf::Texture t_closeBtn;
	static sf::Texture t_pauseBtn;
	static sf::Texture t_settingsBtn;
	static sf::Texture t_slider;

	// Stage Select
	static sf::Texture t_stageNodeBtn;
	static sf::Texture t_armoryBtn;
	// Stage UI
	static sf::Texture t_upgradeWalletBtn;
	static sf::Texture t_fireCannonBtn;
	// Start Menu
	static sf::Texture t_quitBtn;
	static sf::Texture t_startGameBtn;
	// WorkShop
	static sf::Texture t_switchGearBtn;
	static sf::Texture t_speedUpBtn;
	#pragma endregion 
	// END OF BUTTONS REGION ----------------------------------------------

	static sf::Texture t_menuBG1;
	static sf::Texture t_menuBG2;

	static sf::Texture t_statusIcons;
	static std::array<sf::IntRect, 8> r_statusIcons;


	/// <summary>
	/// Right now it doesnt matter, but make sure the first 5 textures of this 
	/// SpriteSheet match the Indexing for the actual UnitAnimationEnum values.
	/// </summary>
	static sf::Texture t_workshopAnimBtns;
	static std::array<sf::IntRect, 7> r_workshopAnimBtns;
	static sf::Texture t_workshopStatsIcons;
	static std::array<sf::IntRect, 8> r_workshopStatsIcons;
	
	static std::array<std::array<sf::Texture, 3>, 3> t_unitSlots;

	static void initialize();
	static void loadTexture(sf::Texture& tex, const std::string& path, bool printErrorMsg = true);

	static void fillTextureRects(std::span<sf::IntRect> out, sf::Vector2u texSize, sf::Vector2i cellSize);
	template<int FRAMES>
	static std::array<sf::IntRect, FRAMES> createTextureRects(sf::Vector2u texSize, sf::Vector2i cellSizes) {
		std::array<sf::IntRect, FRAMES> rects;
		fillTextureRects(std::span(rects), texSize, cellSizes);
		return rects;
	}
	static std::vector<sf::IntRect> createTextureRects(int frames, sf::Vector2u texSize, sf::Vector2i cellSizes);

	static sf::Texture& getUnitSlot(int id, int gear);
	static sf::Texture& getUnitSlot(std::pair<int, int> unit);
};