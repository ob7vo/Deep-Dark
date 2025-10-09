#pragma once
#include "SFML\Graphics.hpp"
class Camera
{
public:
	float zoomLevel;
	sf::View view;
	
	Camera(float zoomLevel);
	sf::View get_view(sf::Vector2u windowSize);
};

