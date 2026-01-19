#include "pch.h"
#include "Lane.h"

Lane::Lane(const nlohmann::json& laneJson, int index) : laneIndex(index)
{
	playerSpawnPoint = laneJson["player_spawn_position"];
	enemySpawnPoint = laneJson["enemy_spawn_position"];
	yPos = laneJson["y_position"];

	if (laneJson.contains("gaps"))
		for (auto& gap : laneJson["gaps"]) {
			if (gap[1] <= gap[0]) std::cout << "Invalid gap" << std::endl;
			else gaps.push_back({ gap[0], gap[1] });
		}

	gapsShapes.reserve(gaps.size());
	gaps.reserve(gaps.size() + 1);
	setShapes();

	playerUnitIndexes.reserve(RESERVED_UNITS);
	enemyUnitIndexes.reserve(RESERVED_UNITS);
}

#pragma region Drawing
void Lane::draw(sf::RenderWindow& window) const {
	window.draw(floor);

	for (auto& shape : gapsShapes)
		window.draw(shape);
}
void Lane::setShapes() {
	floor = sf::RectangleShape({ enemySpawnPoint - playerSpawnPoint, 7.5f });
	floor.setFillColor(sf::Color::Green);
	float X1 = playerSpawnPoint + (enemySpawnPoint - playerSpawnPoint) * .5f;
	floor.setOrigin({ (enemySpawnPoint - playerSpawnPoint) * .5f, 0.f });
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
	return right < playerSpawnPoint - EDGE_EXTENSION || left > enemySpawnPoint + EDGE_EXTENSION;
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
	float xPos = team == UnitConfig::PLAYER_TEAM ? playerSpawnPoint : enemySpawnPoint;
	return { xPos, yPos };
}
float Lane::get_team_boundary(int team) const {
	return team == UnitConfig::PLAYER_TEAM ? playerSpawnPoint + WALL_PADDING : enemySpawnPoint - WALL_PADDING;
}
size_t Lane::get_unit_count(int team) const {
	if (std::abs(team) != 1) return playerUnitIndexes.size() + enemyUnitIndexes.size();
	else return team == UnitConfig::ENEMY_TEAM ? enemyUnitIndexes.size() : playerUnitIndexes.size();
}
std::pair<float, float> Lane::find_closest_gap_ahead(int team, std::pair<float, float> unitHurtboxEdges) const {
	if (gaps.empty()) return { playerSpawnPoint, enemySpawnPoint };

	// The edge of the hurtbox the unit is moving towards (Players move right, Enemies mvoe left)
	float frontHurtboxEdge = team == 1 ? unitHurtboxEdges.second : unitHurtboxEdges.first;

	if (team == 1) {
		// Left -> Right
		for (const auto& gap : gaps) 
			if (gap.first >= frontHurtboxEdge)
				return gap;
	}
	else {
		// Right -> Left
		for (size_t i = gaps.size(); i > 0; --i)
			if (gaps[i].second <= frontHurtboxEdge)
				return gaps[i];
	}

	// No gap ahead. Return fallback
	return { playerSpawnPoint, enemySpawnPoint };
}
float Lane::get_stopping_point(float newX, float stopDist, 
	int team, std::pair<float, float> unitHurtboxEdges) const {
	// No gaps, so the "closest gap ahead" defaulted to the lane boundary
	if (gaps.empty()) return std::clamp(newX, playerSpawnPoint + stopDist, enemySpawnPoint - stopDist);

	const auto [stopLeft, stopRight] = find_closest_gap_ahead(team, unitHurtboxEdges);

	return team == UnitConfig::PLAYER_TEAM ? std::min(newX, stopLeft - stopDist) :
		std::max(newX, stopRight + stopDist);
}