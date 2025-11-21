#include "TextureManager.h"
#include <iostream>

std::array<sf::Texture, 8> TextureManager::t_statusIcons;
std::array<sf::Texture, 8> TextureManager::workshopIcons;
sf::Texture TextureManager::t_defaultUnitSlot;

void TextureManager::initialize() {
	loadTexture(t_defaultUnitSlot, "sprites/defaults/empty_slot.png");

	loadTexture(t_statusIcons[0], effectsPath + "slow_icon.png");
	loadTexture(t_statusIcons[1], effectsPath + "overload_icon.png");
	loadTexture(t_statusIcons[2], effectsPath + "weaken_icon.png");
	loadTexture(t_statusIcons[3], effectsPath + "blind_icon.png");
	loadTexture(t_statusIcons[4], effectsPath + "corrode_icon.png");
	loadTexture(t_statusIcons[5], effectsPath + "short_circuit_icon.png");
	loadTexture(t_statusIcons[6], effectsPath + "virus_icon.png");
	loadTexture(t_statusIcons[7], effectsPath + "strengthen_icon.png");

	loadTexture(workshopIcons[0], iconsPath + "attack_time_icon.png");
	loadTexture(workshopIcons[1], iconsPath + "cost_icon.png");
	loadTexture(workshopIcons[2], iconsPath + "hits_icon.png");
	loadTexture(workshopIcons[3], iconsPath + "hp_icon.png");
	loadTexture(workshopIcons[4], iconsPath + "knockbacks_icon.png");
	loadTexture(workshopIcons[5], iconsPath + "recharge_time_icon.png");
	loadTexture(workshopIcons[6], iconsPath + "sight_range_icon.png");
	loadTexture(workshopIcons[7], iconsPath + "speed_icon.png");
}

void TextureManager::loadTexture(sf::Texture& tex, const std::string path) {
	if (!tex.loadFromFile(path))
		std::cout << "Could not load a texture from the path: [" << path
		<< "]" << std::endl;
}