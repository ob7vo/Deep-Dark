#pragma once
#include "StageEntity.h"
#include <json_fwd.hpp>

constexpr sf::Vector2f TELEPORTER_HITBOX = { 32.f, 32.f };
class Unit;

struct Teleporter : public StageEntity{
	float xDestination = 0;
	int team = 0;
	int connectedLane = 0;
	sf::FloatRect bounds;

	AnimationClip animClip;

	Teleporter(const nlohmann::json& tpJson, sf::Vector2f pos, int lane);
	~Teleporter() override = default;

	void tick(Stage& stage, float dt) override;
	void action(Stage& stage) override;
	void check_units(std::vector<Unit>& units) const;
	void create_animation();
	bool found_valid_target(const Unit& unit) const;

	void draw(sf::RenderWindow& window) const;
};

