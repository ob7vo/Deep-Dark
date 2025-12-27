#include "pch.h"
#include "Teleporter.h"
#include "Stage.h"
#include "EntityTextures.h"
#include "EntityConfigs.h"

Teleporter::Teleporter(const nlohmann::json& tpJson, sf::Vector2f pos, int lane) :
	StageEntity(pos, lane) {
	connectedLane = tpJson["connected_lane"];
	team = tpJson["team"];
	xDestination = tpJson["x_destination"];

	create_animation();
	bounds = { pos - animPlayer.clip->origin, EntityConfigs::Traps::TELEPORTER_HITBOX };
}

void Teleporter::tick(Stage& stage, float dt) {
	animPlayer.update(dt, sprite);
	action(stage);
}
void Teleporter::check_units(std::vector<Unit>& units) const {
	for (auto& unit : units) 
		if (found_valid_target(unit)) 
			unit.movement.push_teleport_request(unit, *this);
}
void Teleporter::action(Stage& stage) {
	// input the units to check for
	if (team == 0) {
		check_units(stage.lanes[laneInd].getAllyUnits(-1));
		check_units(stage.lanes[laneInd].getAllyUnits(1));
	}
	else check_units(stage.lanes[laneInd].getAllyUnits(team));
}

void Teleporter::create_animation() {
	std::string path = "sprites/traps/teleporter.png";
	sf::Vector2i cellSize = { 32,32 };
	sf::Vector2f origin = { 16,32 };
	int frames = 6;
	float rate = 0.15f;

	animClip = AnimationClip(&Textures::Entity::t_teleporter, frames, rate, cellSize, origin, {}, true);
	animPlayer.start(&animClip, sprite);
}
void Teleporter::draw(sf::RenderWindow& window) const {
	sf::RectangleShape hitbox(EntityConfigs::Traps::TELEPORTER_HITBOX);

	hitbox.setPosition(pos);
	hitbox.setFillColor(HITBOX_COLOR);
	hitbox.setOrigin(hitbox.getSize() * 0.5f);

	window.draw(sprite);
	window.draw(hitbox);
}

bool Teleporter::found_valid_target(const Unit& unit) const{
	auto animState = static_cast<int>(unit.anim.get_state());
	bool validAnimation = animState >= 0 && animState <= 5;
	
	return collides(bounds, unit) && !unit.stats->ancient_type() &&
		validAnimation;
}