#pragma once
#include "TextureManager.h"

namespace Textures::Effects {
	extern sf::Texture t_statusIcons;
	extern std::array<sf::IntRect, 12> r_statusIcons;

	void initialize();
}