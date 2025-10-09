#pragma once
#include "Teleporter.h"
#include "Trap.h"
#include "Unit.h"
#include <memory>
#include <vector>
const float EDGE_EXTENSION = 7.5f;
const float WALL_PADDING = 5.f;
const int RESERVED_UNITS = 15;

struct Lane {
	int laneIndex;
	float playerXPos;
	float enemyXPos;
	float yPos;
	std::vector<std::pair<float, float>> gaps; // two X positions
	sf::RectangleShape floor;
	std::vector<sf::RectangleShape> gapsShapes;

	std::vector<Unit> playerUnits; // Vectors cant have references
	std::vector<Unit> enemyUnits;
	std::unique_ptr<Teleporter> enemyTeleporter = nullptr;
	std::unique_ptr<Teleporter> playerTeleporter = nullptr;
	std::unique_ptr<Trap> trap = nullptr;

	Lane(float eX, float pX, float y, std::vector<std::pair<float, float>> gaps, int index)
		: enemyXPos(eX), playerXPos(pX), yPos(y), laneIndex(index), gaps(gaps), floor({ eX-pX,7.5f })
	{
		gapsShapes.reserve(gaps.size());
		gaps.reserve(gaps.size() + 1);
		setShapes();
		playerUnits.reserve(RESERVED_UNITS);
		enemyUnits.reserve(RESERVED_UNITS);
	}

	inline void setShapes() {
		floor.setFillColor(sf::Color::Green);
		float X1 = playerXPos + (enemyXPos - playerXPos) * .5f;
		floor.setOrigin({ (enemyXPos - playerXPos) * .5f, 0.f });
		floor.setPosition({ X1, yPos });

		for (auto& gap : gaps) {
			sf::Vector2f size({ (gap.second - gap.first),7.5f });
			gapsShapes.emplace_back(size);
			float x = gap.first + size.x * .5f;
			gapsShapes.back().setOrigin({size.x * .5f, 0.f});
			gapsShapes.back().setPosition({ x, yPos });
		}
	//	float X = gap.x + (gap.y - gap.x) * .5f;
	}
	inline void add_shape(std::pair<float,float> gap) {
		sf::Vector2f size({ (gap.second - gap.first),7.5f });
		gapsShapes.emplace_back(size);
		float x = gap.first + size.x * .5f;
		gapsShapes.back().setOrigin({ size.x * .5f, 0.f });
		gapsShapes.back().setPosition({ x, yPos });
	}
	Teleporter* get_teleporter(int team) {
		return team == 1 ? playerTeleporter.get() : enemyTeleporter.get();
	}
	inline std::vector<Unit>& get_targets(int team) { return team == PLAYER_TEAM ? enemyUnits : playerUnits; }
	inline std::vector<Unit>& get_source(int team) { return team == ENEMY_TEAM ? enemyUnits : playerUnits; }

	inline sf::Vector2f get_spawn_pos(int team) {
		float xPos = team == PLAYER_TEAM ? playerXPos : enemyXPos;
		return { xPos, yPos };
	}
	inline bool out_of_lane(float xPos) const {
		return xPos < playerXPos - EDGE_EXTENSION || xPos > enemyXPos + EDGE_EXTENSION;
	}
	inline float get_wall(int team) {
		return team == PLAYER_TEAM ? playerXPos + WALL_PADDING : enemyXPos - WALL_PADDING;
	}
	inline bool within_gap(float xPos) {
		if (xPos < playerXPos - EDGE_EXTENSION || xPos > enemyXPos + EDGE_EXTENSION)
			return true;

		for (auto& gap : gaps) 
			if (xPos > gap.first && xPos < gap.second)
				return true;
		
		return false;
	}
	inline void draw(sf::RenderWindow& window) {
		window.draw(floor);
		//window.draw(gapShape);
		for (auto& shape : gapsShapes)
			window.draw(shape);

		if (playerTeleporter) window.draw(playerTeleporter->shape);
		if (enemyTeleporter) window.draw(enemyTeleporter->shape);
	}
};

