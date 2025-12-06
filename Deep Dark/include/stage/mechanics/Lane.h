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

	inline std::vector<Unit>& getOpponentUnits(int team) { return team == PLAYER_TEAM ? enemyUnits : playerUnits; }
	inline std::vector<Unit>& getAllyUnits(int team) { return team == ENEMY_TEAM ? enemyUnits : playerUnits; }
	size_t get_unit_count(int team = 0) const;
	sf::Vector2f get_spawn_pos(int team) const;
	float get_team_boundary(int team) const;
	std::pair<float, float> get_lane_boundaries() const 
	{ return { get_team_boundary(1), get_team_boundary(-1) }; }

	bool out_of_lane(float left, float right) const;
	bool within_gap(float leftHurtboxEdge, float rightHurtboxEdge) const;
};

