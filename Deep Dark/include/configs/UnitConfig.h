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
	/// <summary> Get the specific unit files. (The ID folder path + the gear folder) </summary>
	std::string getUnitGearPath(int id, int gear, bool throwError = true);
	nlohmann::json createUnitJson(int id, int gear);
	nlohmann::json createSummonJson(int id);
	sf::Texture createSlotTexture(int id, int gear);
	/// <summary> Gets the highest gear leel Unit #id has  </summary>
	int getMaxGear(int id);

	// Overloads
	std::string getUnitGearPath(std::pair<int, int> unit);
	nlohmann::json createUnitJson(std::pair<int, int> unit);
	sf::Texture createSlotTexture(std::pair<int, int> unit);

	bool shouldFlipSprite(int id);
	sf::Color getGearColor(int id, int gear);
};