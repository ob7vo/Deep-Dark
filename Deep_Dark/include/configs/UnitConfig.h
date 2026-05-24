#pragma once
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Color.hpp>
#include <json_fwd.hpp>
#include "UnitConfigConstants.h"

const float GUARANTEED_CHANCE = 100.0f;

// Constant data of game elements relating to Units
// Also has getters for Unit Jsons.
namespace UnitConfig {
	/// <summary> Get only the general folder of the Unit </summary>
	std::string getUnitFolderPath(int id);
	nlohmann::json createUnitJson(int id);
	nlohmann::json createSummonJson(int id);
	sf::Texture createSlotTexture(int id);
	/// <summary> Gets the highest gear leel Unit #id has  </summary>

	bool shouldFlipSprite(int id);
	sf::Color getUnitColor(int id);
};