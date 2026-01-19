#pragma once
#include "UnitConfigConstants.h"

const float EDGE_EXTENSION = 7.5f;
const float WALL_PADDING = 5.f;
const int RESERVED_UNITS = 15;

struct Lane {
	int laneIndex = 0;
	float playerSpawnPoint = 0;
	float enemySpawnPoint = 0;
	float yPos = 0;

	std::vector<std::pair<float, float>> gaps; // two X positions
	sf::RectangleShape floor;
	std::vector<sf::RectangleShape> gapsShapes;

	std::vector<size_t> playerUnitIndexes; // Vectors cant have references
	int maxPlayers = RESERVED_UNITS;
	std::vector<size_t> enemyUnitIndexes;
	int maxEnemies = RESERVED_UNITS;

	Lane(const nlohmann::json& laneJson, int index);

	void draw(sf::RenderWindow& window) const;
	void setShapes();
	void add_shape(std::pair<float, float> gap);

	inline std::vector<size_t>& getOpponentUnits(int team) { return team == UnitConfig::PLAYER_TEAM ? enemyUnitIndexes : playerUnitIndexes; }
	inline std::vector<size_t>& getAllyUnits(int team) { return team == UnitConfig::ENEMY_TEAM ? enemyUnitIndexes : playerUnitIndexes; }
	inline const std::vector<size_t>& getOpponentUnits(int team) const { return team == UnitConfig::PLAYER_TEAM ? enemyUnitIndexes : playerUnitIndexes; }
	inline const std::vector<size_t>& getAllyUnits(int team) const { return team == UnitConfig::ENEMY_TEAM ? enemyUnitIndexes : playerUnitIndexes; }

	size_t get_unit_count(int team = 0) const;
	sf::Vector2f get_spawn_pos(int team) const;
	float get_team_boundary(int team) const;
	inline std::pair<float, float> get_lane_boundaries() const 
	{ return { get_team_boundary(1), get_team_boundary(-1) }; }
	std::pair<float, float> find_closest_gap_ahead(int team, std::pair<float, float> unitHurtboxEdges) const;
	float get_stopping_point(float newX, float stoppingDistance, int team, std::pair<float, float> unitHurtboxEdges) const;

	bool out_of_lane(float left, float right) const;
	bool within_gap(float leftHurtboxEdge, float rightHurtboxEdge) const;
};

