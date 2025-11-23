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
	int laneIndex = 0;
	float playerXPos = 0;
	float enemyXPos = 0;
	float yPos = 0;

	std::vector<std::pair<float, float>> gaps; // two X positions
	sf::RectangleShape floor;
	std::vector<sf::RectangleShape> gapsShapes;

	std::vector<Unit> playerUnits; // Vectors cant have references
	std::vector<Unit> enemyUnits;
	std::unique_ptr<Teleporter> enemyTeleporter = nullptr;
	std::unique_ptr<Teleporter> playerTeleporter = nullptr;
	std::unique_ptr<Trap> trap = nullptr;

	Lane(const nlohmann::json& laneJson, int index) : laneIndex(index)
	{
		playerXPos = laneJson["player_spawn_position"];
		enemyXPos = laneJson["enemy_spawn_position"];
		yPos = laneJson["y_position"];

		if (laneJson.contains("gaps"))
			for (auto& gap : laneJson["gaps"]) {
				if (gap[1] <= gap[0]) std::cout << "Invalid gap" << std::endl;
				else gaps.push_back({ gap[0], gap[1] });
			}

		gapsShapes.reserve(gaps.size());
		gaps.reserve(gaps.size() + 1);
		setShapes();

		playerUnits.reserve(RESERVED_UNITS);
		enemyUnits.reserve(RESERVED_UNITS);
	}

	inline void setShapes() {
		floor = sf::RectangleShape({ enemyXPos - playerXPos, 7.5f });
		floor.setFillColor(sf::Color::Green);
		float X1 = playerXPos + (enemyXPos - playerXPos) * .5f;
		floor.setOrigin({ (enemyXPos - playerXPos) * .5f, 0.f });
		floor.setPosition({ X1, yPos });

		for (auto const [leftEdge, rightEdge] : gaps) {
			sf::Vector2f size({ (rightEdge - leftEdge),7.5f });
			gapsShapes.emplace_back(size);

			float x = leftEdge + size.x * .5f;
			gapsShapes.back().setOrigin({size.x * .5f, 0.f});
			gapsShapes.back().setPosition({ x, yPos });
		}
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

	inline sf::Vector2f get_spawn_pos(int team) const {
		float xPos = team == PLAYER_TEAM ? playerXPos : enemyXPos;
		return { xPos, yPos };
	}
	inline bool out_of_lane(float xPos) const {
		return xPos < playerXPos - EDGE_EXTENSION || xPos > enemyXPos + EDGE_EXTENSION;
	}
	inline float get_wall(int team) const {
		return team == PLAYER_TEAM ? playerXPos + WALL_PADDING : enemyXPos - WALL_PADDING;
	}
	inline bool within_gap(float xPos) const{
		if (xPos < playerXPos - EDGE_EXTENSION || xPos > enemyXPos + EDGE_EXTENSION)
			return true;

		for (auto const [leftEdge, rightEdge] : gaps)
			if (xPos > leftEdge && xPos < rightEdge)
				return true;
		
		return false;
	}

	inline void draw(sf::RenderWindow& window) {
		window.draw(floor);

		for (auto& shape : gapsShapes)
			window.draw(shape);

		if (playerTeleporter) window.draw(playerTeleporter->shape);
		if (enemyTeleporter) window.draw(enemyTeleporter->shape);
	}
	inline size_t get_unit_count(int team) {
		if (std::abs(team) != 1) return playerUnits.size() + enemyUnits.size();
		else return get_source(team).size();
	}
};

