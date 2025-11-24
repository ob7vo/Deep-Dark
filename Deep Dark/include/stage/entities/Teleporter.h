#pragma once
#include "json.hpp"
#include "StageEntity.h"

const float TELEPORTER_AREA = 7.5f;
const float TELEPORTER_HEIGHT = 15.f;

class Unit;

struct Teleporter : public StageEntity{
	float xDestination = 0;
	int team = 0;
	int connectedLane = 0;

	Teleporter(const nlohmann::json& tpJson, sf::Vector2f pos, int lane) :
		StageEntity(pos, lane) {
		connectedLane = tpJson["connected_lane"];
		team = tpJson["team"];
		xDestination = tpJson["x_destination"];
	}
	~Teleporter() override = default;

	void tick(Stage& stage, float dt) override;
	void action(Stage& stage) override;
	void check_units(std::vector<Unit>& units);
	void create_animation();

	inline bool within_range(float x, float y) const{
		return (x >= pos.x - TELEPORTER_AREA && x <= pos.x + TELEPORTER_AREA) &&
			(y <= pos.y + TELEPORTER_AREA && y >= pos.y - TELEPORTER_HEIGHT);
	}
};

