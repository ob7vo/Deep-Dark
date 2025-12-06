#include "pch.h"
#include "Lane.h"

Lane::Lane(const nlohmann::json& laneJson, int index) : laneIndex(index)
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

#pragma region Drawing
void Lane::draw(sf::RenderWindow& window) const {
	window.draw(floor);

	for (auto& shape : gapsShapes)
		window.draw(shape);
}
void Lane::setShapes() {
	floor = sf::RectangleShape({ enemyXPos - playerXPos, 7.5f });
	floor.setFillColor(sf::Color::Green);
	float X1 = playerXPos + (enemyXPos - playerXPos) * .5f;
	floor.setOrigin({ (enemyXPos - playerXPos) * .5f, 0.f });
	floor.setPosition({ X1, yPos });

	for (auto const [leftEdge, rightEdge] : gaps) {
		sf::Vector2f size({ (rightEdge - leftEdge),7.5f });
		gapsShapes.emplace_back(size);

		float x = leftEdge + size.x * .5f;
		gapsShapes.back().setOrigin({ size.x * .5f, 0.f });
		gapsShapes.back().setPosition({ x, yPos });
	}
}
void Lane::add_shape(std::pair<float, float> gap) {
	sf::Vector2f size({ (gap.second - gap.first),7.5f });
	gapsShapes.emplace_back(size);
	float x = gap.first + size.x * .5f;
	gapsShapes.back().setOrigin({ size.x * .5f, 0.f });
	gapsShapes.back().setPosition({ x, yPos });
}
#pragma endregion

#pragma region Checks
bool Lane::out_of_lane(float left, float right) const {
	return right < playerXPos - EDGE_EXTENSION || left > enemyXPos + EDGE_EXTENSION;
}
bool Lane::within_gap(float leftHurtboxEdge, float rightHurtboxEdge) const {
	if (out_of_lane(leftHurtboxEdge, rightHurtboxEdge))
		return true;

	for (const auto& [gapLeft, gapRight] : gaps)
		if (leftHurtboxEdge > gapLeft && rightHurtboxEdge < gapRight)
			return true;

	return false;
}
#pragma endregion

// Getters
sf::Vector2f Lane::get_spawn_pos(int team) const {
	float xPos = team == PLAYER_TEAM ? playerXPos : enemyXPos;
	return { xPos, yPos };
}
float Lane::get_team_boundary(int team) const {
	return team == PLAYER_TEAM ? playerXPos + WALL_PADDING : enemyXPos - WALL_PADDING;
}
size_t Lane::get_unit_count(int team) const {
	if (std::abs(team) != 1) return playerUnits.size() + enemyUnits.size();
	else return team == ENEMY_TEAM ? enemyUnits.size() : playerUnits.size();
}