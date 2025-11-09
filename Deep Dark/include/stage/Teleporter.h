#pragma once
const float TELEPORTER_AREA = 7.5f;
const float TELEPORTER_HEIGHT = 15.f;

struct Teleporter {
	float xPos;
	float xDestination;
	float yPos;
	int connectedLane;
	sf::RectangleShape shape;

	Teleporter(int lane, float xPos, float xDestination, float y)
		: connectedLane(lane), xPos(xPos), xDestination(xDestination), shape({15.f,7.5f}), yPos(y) {
		shape.setOrigin({ 7.5f, 7.5f });
		shape.setPosition({ xPos, y });
		shape.setFillColor(sf::Color::Magenta);
	}
	inline bool check_if_on_teleporter(float x, float y) {
		return (x >= xPos - TELEPORTER_AREA && x <= xPos + TELEPORTER_AREA) &&
			(y <= yPos + TELEPORTER_AREA && y >= yPos - TELEPORTER_HEIGHT);
	}
};

