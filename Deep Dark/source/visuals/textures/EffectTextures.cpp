#include "pch.h"
#include "EffectTextures.h"
#include "Utils.h"

using namespace FolderPaths;

namespace Textures::Effects {
	sf::Texture t_statusIcons = createTexture(path(effectsPath, "status_icons.png"));
	std::array<sf::IntRect, 8> r_statusIcons;

	void initialize() {
		r_statusIcons = createTextureRects<8>(t_statusIcons.getSize(), { 32, 32 });
	}
}