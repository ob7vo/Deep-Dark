#pragma once
#include "StageEntity.h"

const float TELEPORTER_AREA = 32.f;
const float TELEPORTER_HEIGHT = 32.f;

class Unit;

struct Teleporter : public StageEntity{
	float xDestination = 0;
	int team = 0;
	int connectedLane = 0;

	Teleporter(const nlohmann::json& tpJson, sf::Vector2f pos, int lane);
	~Teleporter() override = default;

	void tick(Stage& stage, float dt) override;
	void action(Stage& stage) override;
	void check_units(std::vector<Unit>& units) const;
	void create_animation();
	bool found_valid_target(const Unit& unit) const;

	void draw(sf::RenderWindow& window) const;

	inline bool within_range(sf::Vector2f unitPos) const{
		return (unitPos.x >= pos.x - TELEPORTER_AREA && unitPos.x <= pos.x + TELEPORTER_AREA) &&
			(unitPos.y <= pos.y + TELEPORTER_HEIGHT && unitPos.y >= pos.y - TELEPORTER_HEIGHT);
	}
};

