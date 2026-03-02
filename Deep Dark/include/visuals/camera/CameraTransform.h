#pragma once
#include "Bounds.h"
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/System/Vector2.hpp>

class Camera;

struct CameraTransform {
	Camera& cam;

	Bounds limits;
	bool locked = true;

	float zoomLevel = 1;
	sf::Vector2f worldPos;

	CameraTransform(Camera& cam);

	void zoom(sf::Event::MouseWheelScrolled scroll);
	void set_zoom(float zoomOffset);
	void zoom(sf::Keyboard::Key key);
	void apply_velocity(float deltaTime);
	void move(sf::Keyboard::Key key);
	void click_and_drag();

	sf::Vector2f get_pos_within_bounds(sf::Vector2f targetPos) const;
	void update_pos(sf::Vector2f pos);
};