#pragma once
#include "json.hpp"

const float TELEPORTER_AREA = 7.5f;
const float TELEPORTER_HEIGHT = 15.f;

struct Teleporter {
	float xPos = 0;
	float xDestination = 0;
	float yPos = 0;

	int connectedLane;
	sf::RectangleShape shape;

	Teleporter(const nlohmann::json& tpJson, float y) : shape({15.f,7.5f}), yPos(y) {
		connectedLane = tpJson["connected_lane"];
		xPos = tpJson["x_position"];
		xDestination = tpJson["x_destination"];

		shape.setOrigin({ 7.5f, 7.5f });
		shape.setPosition({ xPos, y });
		shape.setFillColor(sf::Color::Magenta);
	}
	inline bool check_if_on_teleporter(float x, float y) {
		return (x >= xPos - TELEPORTER_AREA && x <= xPos + TELEPORTER_AREA) &&
			(y <= yPos + TELEPORTER_AREA && y >= yPos - TELEPORTER_HEIGHT);
	}
};

