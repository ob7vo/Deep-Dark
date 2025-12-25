#include "pch.h"
#include "Base.h"
#include "Stage.h"
#include "BaseTextures.h"

using json = nlohmann::json;

const int ENEMY_BASE = -1;
const int PLAYER_BASE = 1;
const float BASE_Y_POS = 350;

Base::Base(const json& baseJson, int team) : 
	StageEntity({0.f,0.f}, 0), 
	team(team)
{
	/*
	* If I set a sprites Color.a (opacity) to 0, does this make drawing time effectively 0? 
	* Does the compiler realize it can just not draw the object?
	*/
	hp = maxHp = baseJson["hp"];
	pos = { baseJson["x_position"], BASE_Y_POS };

	if (baseJson.contains("cannon_json_path")) {
		create_cannon(baseJson["cannon_json_path"].get<std::string>(), 
			baseJson.value("magnification", 1.f));
	}
	else sprite.setTexture(Textures::Base::t_base1);
	
	sprite.setPosition(pos);
}
void Base::create_cannon(const std::string& path, float magnification) {
	std::ifstream file(path);
	json baseFile = json::parse(file);

	cannonTimer = baseFile["cannon_timer"];
	sightRange = baseFile.value("sight_range", 0.0f);

	switch (baseFile["type"].get<int>()) {
	case 1:
		cannon = std::make_unique<WaveCannon>(baseFile, magnification);
		break;
	case 2:
		cannon = std::make_unique<FireWallCannon>(baseFile, magnification);
		break;
	case 3:
		cannon = std::make_unique<OrbitalCannon>(baseFile, magnification);
		std::cout << "Creating Orbital Strike Cannon" << std::endl;
		break;
	case 4:
		cannon = std::make_unique<AreaCannon>(baseFile, magnification);
		break;
	default: 
		throw std::runtime_error("Invalid Cannon Type");
	}

	cannon->create_animation();
	animPlayer.start(&cannon->animClip, sprite);
	cannon->pos = pos;
}

void Base::take_damage(Stage* stage, int dmg) {
	hp -= dmg;

	if (team == ENEMY_TEAM)
		stage->break_spawner_thresholds();

	if (hp <= 0) stage->destroy_base(team);
}
bool Base::try_fire_cannon() {
	if (!cannon) { // Has no cannon
		cooldown = 999.f;
		std::cout << "Base has no Cannon to fire" << std::endl;
		return false;
	}
	else if (on_cooldown()) return false; 

	cooldown = cannonTimer;

	animating = true;
	animPlayer.reset(sprite);

	return true;
}

void Base::tick(Stage& stage, float deltaTime) {
	if (!cannon) return;

	if (animating) update_animation(stage, deltaTime);

	cooldown -= deltaTime;

	// Below this wil be the base checking fro autmatic firing.
	// ON in-range enemies, this only applies to Enemy Bases.
	if (team == PLAYER_BASE || on_cooldown()) return;

	for (const auto& lane : stage.lanes) {
		for (const auto& unit : lane.playerUnits)
			if (enemy_in_range(unit.get_pos().x)) {
				try_fire_cannon();
				return;
			};
	}
}
AnimationEvent Base::update_animation(Stage& stage, float deltaTime) {
	auto events = animPlayer.update(deltaTime, sprite);

	if (any(events & AnimationEvent::ATTACK))
		cannon->fire(stage);
	if (any(AnimationEvent::FINAL_FRAME & events)) {
		animPlayer.reset(sprite);
		animating = false;
	}

	return events;
}
void Base::draw(sf::RenderWindow& window) const {
	window.draw(sprite);
}

