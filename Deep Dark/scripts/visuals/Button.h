#pragma once
#include <SFML\Graphics.hpp>
#include <functional>

class Button
{
	sf::Vector2f viewportPos;
	sf::Vector2f pos;
	sf::Vector2f bounds;
	sf::Color baseColor;
public:
	std::function<void()> onClick;
	sf::RectangleShape shape;
	bool hovering = 0;

	Button(sf::Vector2f pos, sf::Vector2f bounds, sf::Color color, std::function<void()> onClick);

	void check_mouse_hover(sf::Vector2f mousePos);
	bool is_hovering(sf::Vector2f mousePos);

	void draw(sf::RenderWindow& window);
};

