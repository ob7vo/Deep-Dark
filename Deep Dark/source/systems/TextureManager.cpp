#include "pch.h"
#include "TextureManager.h"
#include "UnitData.h"
#include "Utils.h"
#include "BaseTextures.h"
#include "UITextures.h"
#include "EffectTextures.h"
#include "EntityTextures.h"

using namespace FolderPaths;

namespace Textures {
	void initializeAll() {
		Textures::UI::initialize();
		Textures::Effects::initialize();
	}

	bool loadTexture(sf::Texture& tex, std::string_view path) {
		if (!tex.loadFromFile(path)) {
			tex = defTex;
			return false;
		}

		return true;
	}
	sf::Texture createTexture(std::string_view path) {
		sf::Texture tex;
		loadTexture(tex, path);
		return tex;
	}

	void fillTextureRects(std::span<sf::IntRect> spanRects, sf::Vector2u texSize, sf::Vector2i cellSize) {
		if (cellSize.x <= 0 || cellSize.y <= 0)
			throw InvalidCellSizeError();

		size_t FRAMES = spanRects.size();
		int columns = texSize.x / cellSize.x;
		int rows = texSize.y / cellSize.y;
		int totalCells = columns * rows;

		if (texSize.x % cellSize.x != 0 || texSize.y % cellSize.y != 0)
			throw InvalidCellSizeError(cellSize, texSize);
		if (FRAMES > totalCells)
			throw InvalidCellSizeError(FRAMES, totalCells);


		int frame = 0;
		for (int row = 0; row < rows && frame < FRAMES; row++) {
			for (int col = 0; col < columns && frame < FRAMES; col++) {
				sf::IntRect rect({ col * cellSize.x, row * cellSize.y }, cellSize);
				spanRects[frame++] = rect;
			}
		}

		if (frame != FRAMES)
			std::cerr << "Warning: Only loaded " << frame << " of " << FRAMES << " frames" << std::endl;
	}
	std::vector<sf::IntRect> createTextureRects(int frames, sf::Vector2u texSize, sf::Vector2i cellSizes) {
		std::vector<sf::IntRect> rects(frames);
		fillTextureRects(std::span(rects), texSize, cellSizes);
		return rects;
	}
}