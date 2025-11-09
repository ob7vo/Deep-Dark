#include "TextureManager.h"

std::array<sf::Texture, 8> TextureManager::t_statusIcons;

void TextureManager::initialize() {
	t_statusIcons[0].loadFromFile(effectsPath + "slow_icon.png");
	t_statusIcons[1].loadFromFile(effectsPath + "overload_icon.png");
	t_statusIcons[2].loadFromFile(effectsPath + "weaken_icon.png");
	t_statusIcons[3].loadFromFile(effectsPath + "blind_icon.png");
	t_statusIcons[4].loadFromFile(effectsPath + "corrode_icon.png");
	t_statusIcons[5].loadFromFile(effectsPath + "short_circuit_icon.png");
	t_statusIcons[6].loadFromFile(effectsPath + "virus_icon.png");
	t_statusIcons[7].loadFromFile(effectsPath + "strengthen_icon.png");
}