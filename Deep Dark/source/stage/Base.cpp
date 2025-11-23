#include "Base.h"
#include "Stage.h"
using json = nlohmann::json;

const int ENEMY_BASE = -1;
const int PLAYER_BASE = 1;
const float BASE_Y_POS = 350;

Base::Base(const json& stageFile, int team) : team(team), sprite(get_default_texture()) {
	std::string baseString = team == 1 ? "player_base" : "enemy_base";
	cannonAnimation = nullptr;

	hp = maxHp = stageFile[baseString]["hp"];
	pos = { stageFile[baseString]["x_position"], BASE_Y_POS };
	sprite.setPosition(pos);
	sprite.setOrigin({ 2.5f, 2.5f });

	if (stageFile[baseString].contains("cannon_json_path"))
		create_cannon(stageFile[baseString]["cannon_json_path"], stageFile[baseString].value("magnification",1.f));
	else {
		cannon = nullptr;
		//(void)texture.loadFromFile(stageFile[baseString]["sprite_path"]);
		Animation* ani = BaseCannon::ga_ptr();
		sprite.setTexture(ani->texture);
		sprite.setTextureRect(ani->frames[0].rect);
	}
}
void Base::create_cannon(std::string path, float magnification) {
	std::ifstream file(path);
	//"configs/base_data/player_bases/p_wave_base.json"	std::string
	//.\configs\base_data\player_bases\p_wave_base.json
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
	//Animation* ani = cannon->get_cannon_animation_ptr();
	Animation* ani = BaseCannon::ga_ptr();
	ani->reset(sprite);
	cannon->pos = pos;
}
void Base::take_damage(int dmg) {
	tookDmgThisFrame = team == -1; // this variable doesnt matter to player base

	hp -= dmg;
}
bool Base::try_fire_cannon() {
	if (!cannon) {
		cooldown = 999.f;
		std::cout << "Base has no Cannon to fire" << std::endl;
		return false;
	}
	else if (on_cooldown()) return false;

	cooldown = cannonTimer;
	//cannonAnimation = cannon->get_cannon_animation_ptr();
	cannonAnimation = BaseCannon::ga_ptr();
	cannonAnimation->reset(sprite);

	return true;
}
void Base::tick(Stage& stage, float deltaTime) {
	if (cannonAnimation) {
		auto events = cannonAnimation->update(deltaTime, sprite);

		if (Animation::check_for_event(AnimationEvent::ATTACK, events) && cannon)
			cannon->fire(stage);
		if (Animation::check_for_event(AnimationEvent::FINAL_FRAME, events)) {
			sprite.setTextureRect(cannonAnimation->frames[0].rect);
			cannonAnimation = nullptr;
		}
	}

	cooldown -= deltaTime;
	if (team == PLAYER_BASE || !cannon || on_cooldown()) return;

	for (auto& lane : stage.lanes)
		for (auto& unit : lane.playerUnits)
			if (enemy_in_range(unit.get_pos().x)) {
				try_fire_cannon();
				return;
			};
}

