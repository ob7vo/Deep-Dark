#include "pch.h"
#include "BaseTextures.h"
#include "Utils.h"

using namespace FolderPaths;

namespace Textures::Base {
	sf::Texture t_base1 = createTexture(path(baseSpritesPath, "green_base.png"));
	sf::Texture t_base2 = createTexture(path(baseSpritesPath, "toxic_base.png"));
}