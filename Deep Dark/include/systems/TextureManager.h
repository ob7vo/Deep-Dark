#pragma once
#include <SFML/Graphics.hpp>

const sf::Texture defTex("sprites/defaults/defaultTexture.png");
const std::string effectsPath = "sprites/effects/";
const std::string workshopPath = "sprites/ui/workshop/";
const std::string iconsPath = "sprites/icons/";
const std::string defaultsPath = "sprites/defaults/";
const std::string uiPath = "sprites/ui/";

struct TextureManager {
	static const sf::Texture dTex;
	static sf::Texture t_defaultUnitSlot;

	// Status Affects
	static std::array<sf::Texture, 8> t_statusIcons;
	static std::array<sf::Texture, 8> workshopIcons;

	static void initialize();
	static void loadTexture(sf::Texture& tex, const std::string path);
};