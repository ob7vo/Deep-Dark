#include "pch.h"
#include "Base.h"
#include "Stage.h"

using json = nlohmann::json;

const int ENEMY_BASE = -1;
const int PLAYER_BASE = 1;
const float BASE_Y_POS = 350;

Base::Base(const json& baseJson, int team) : StageEntity({}, 0), team(team) {
	hp = maxHp = baseJson["hp"];
	pos = { baseJson["x_position"], BASE_Y_POS };

	sprite.setPosition(pos);
	sprite.setOrigin({ 2.5f, 2.5f });

	if (baseJson.contains("cannon_json_path"))
		create_cannon(baseJson["cannon_json_path"], baseJson.value("magnification",1.f));
	else {
		cannon = nullptr;
		//(void)texture.loadFromFile(baseJson["sprite_path"]);
		sprite.setTexture(TextureManager::t_base1);
	}
}
void Base::create_cannon(std::string path, float magnification) {
	std::ifstream file(path);
	json baseFile = json::parse(file);

	cannonTimer = baseFile["cannon_timer"];
	sightRange = baseFile.value("sight_range", 0.0f);
	int type = baseFile["type"];

	switch (type) {
	case 1:
		cannon = std::make_unique<WaveCannon>(baseFile, magnification);
		break;
	case 2:
		cannon = std::make_unique<FireWallCannon>(baseFile, magnification);
		break;
	case 3:
		cannon = std::make_unique<OrbitalCannon>(baseFile, magnification);
		break;
	case 4:
		cannon = std::make_unique<AreaCannon>(baseFile, magnification);
		break;
	}

	anim = cannon->create_animation();
	anim.reset(sprite);
	cannon->pos = pos;
}

void Base::take_damage(Stage* stage, int dmg) {
	hp -= dmg;

	if (team == ENEMY_TEAM)
		stage->break_spawner_thresholds();
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
	anim.reset(sprite);

	return true;
}

void Base::tick(Stage& stage, float deltaTime) {
	if (animating) update_animation(stage, deltaTime);

	cooldown -= deltaTime;
	if (team == PLAYER_BASE || !cannon || on_cooldown()) return;

	for (auto& lane : stage.lanes)
		for (auto& unit : lane.playerUnits)
			if (enemy_in_range(unit.get_pos().x)) {
				try_fire_cannon();
				return;
			};
}
int Base::update_animation(Stage& stage, float deltaTime) {
	int events = anim.update(deltaTime, sprite);

	if (Animation::check_for_event(AnimationEvent::ATTACK, events) && cannon)
		cannon->fire(stage);
	if (Animation::check_for_event(AnimationEvent::FINAL_FRAME, events)) {
		sprite.setTextureRect(anim.frames[0].rect);
		animating = false;
	}

	return events;
}

