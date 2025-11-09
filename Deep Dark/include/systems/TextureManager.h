#include "SFML\Graphics.hpp"

const sf::Texture defTex("sprites/defaults/defaultTexture.png");
const std::string effectsPath = "sprites/effects/";
const std::string defaultsPath = "sprites/defaults/";
const std::string uiPath = "sprites/ui/";

struct TextureManager {

	// Status Affects;
	static std::array<sf::Texture, 8> t_statusIcons;
	//const sf::Texture 

	static void initialize();
};