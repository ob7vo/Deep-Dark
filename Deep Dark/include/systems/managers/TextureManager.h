#pragma once
#include <SFML\Graphics\Texture.hpp>
#include <SFML\Graphics\Rect.hpp>
#include <span>

const sf::Texture defTex("sprites/defaults/defaultTexture.png");

namespace Textures {
	void initializeAll();
	bool loadTexture(sf::Texture& tex, std::string_view path);
	sf::Texture createTexture(std::string_view path);

	void fillTextureRects(std::span<sf::IntRect> out, sf::Vector2u texSize, sf::Vector2i cellSize);
	template<int FRAMES>
	inline std::array<sf::IntRect, FRAMES> createTextureRects(sf::Vector2u texSize, sf::Vector2i cellSizes) {
		std::array<sf::IntRect, FRAMES> rects;
		fillTextureRects(std::span(rects), texSize, cellSizes);
		return rects;
	}
	std::vector<sf::IntRect> createTextureRects(int frames, sf::Vector2u texSize, sf::Vector2i cellSizes);
}