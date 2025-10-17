#pragma once
#include <SFML/Graphics.hpp>
#include <ButtonManager.h>

const sf::Font baseFont("fonts/KOMIKAX_.ttf");

class Menu
{
	ButtonManager buttonManager;
public:
	Menu() = default;
	~Menu() = default;
};

struct StageUI : public Menu {
	sf::Text partsCountText = sf::Text(baseFont);
	sf::Text bagUpgradeCostText = sf::Text(baseFont);
	sf::Text clearedChallengesText = sf::Text(baseFont);

	StageUI() = default;
};

