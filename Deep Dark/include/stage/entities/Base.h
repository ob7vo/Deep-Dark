#pragma once
#include "BaseCannon.h"
#include "Animation.h"
#include "StageEntity.h"
#include <memory>

struct Stage;

class Base : public StageEntity
{
private:
	int team = 0;
	int maxHp = 1;
	int hp = 1;

	float cooldown = 0;
	float cannonTimer = 0;
	float sightRange = 0; // only for enemy bases

public:
	std::unique_ptr<BaseCannon> cannon;

	Base() : StageEntity({}, 0) {};
	Base(const nlohmann::json& stageFile, int team);
	void create_cannon(std::string path, float mag);

	void take_damage(Stage* stage, int dmg);
	bool try_fire_cannon();

	void tick(Stage& stage, float deltaTime) override;
	int update_animation(Stage& stage, float deltaTime) override;
	inline void draw(sf::RenderWindow& window) const { window.draw(sprite); }

	inline float get_hp_percentage() const {
		if (hp <= 0 || maxHp <= 0) return 0.f;
		else return static_cast<float>(hp) / static_cast<float>(maxHp);
	}

	inline bool enemy_in_range(float xPos) const {
		float dist = (xPos - pos.x) * static_cast<float>(team);
		return dist >= 0 && dist <= sightRange;
	}
	inline bool on_cooldown() const { return cooldown > 0.f; }
};

