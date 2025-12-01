#pragma once
#include "Unit.h"

const float EDGE_EXTENSION = 7.5f;
const float WALL_PADDING = 5.f;
const int RESERVED_UNITS = 15;

struct Lane {
	int laneIndex = 0;
	float playerXPos = 0;
	float enemyXPos = 0;
	float yPos = 0;

	std::vector<std::pair<float, float>> gaps; // two X positions
	sf::RectangleShape floor;
	std::vector<sf::RectangleShape> gapsShapes;

	std::vector<Unit> playerUnits; // Vectors cant have references
	std::vector<Unit> enemyUnits;

	Lane(const nlohmann::json& laneJson, int index);

	void draw(sf::RenderWindow& window) const;
	void setShapes();
	void add_shape(std::pair<float, float> gap);

	inline std::vector<Unit>& get_targets(int team) { return team == PLAYER_TEAM ? enemyUnits : playerUnits; }
	inline std::vector<Unit>& get_source(int team) { return team == ENEMY_TEAM ? enemyUnits : playerUnits; }
	size_t get_unit_count(int team = 0) const;
	sf::Vector2f get_spawn_pos(int team) const;
	float get_wall(int team) const;

	bool out_of_lane(float xPos) const;
	bool within_gap(float xPos) const;
};

