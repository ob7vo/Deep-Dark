#pragma once
#include "UnitData.h"
#include <SFML/System/Vector2.hpp>

struct Stage;
struct BaseStats;
struct Animation;
class Unit;
class Base;

struct BaseCannon
{
	sf::Vector2f pos;
	UnitStats cannonStats;
	int team = 1;

	BaseCannon(const nlohmann::json& baseJson, float magnification);
	virtual ~BaseCannon() = default;

	virtual void fire(Stage& stage) = 0;
	virtual Animation* get_cannon_animation_ptr() = 0;
	static void init_animations();
	static Animation* ga_ptr();
};
struct WaveCannon : public BaseCannon {
public:
	Augment shockWave;

	WaveCannon(const nlohmann::json& baseJson, float magnification);
	~WaveCannon() = default;

	void fire(Stage& stage) override;
	Animation* get_cannon_animation_ptr() override;
};
struct FireWallCannon : public BaseCannon {
	Augment fireWall;

	FireWallCannon(const nlohmann::json& baseJson, float magnification);
	~FireWallCannon() = default;

	void fire(Stage& stage) override;
	Animation* get_cannon_animation_ptr() override;
};
struct OrbitalCannon : public BaseCannon {
	//spawns 4 equally spaced orbital strikes
	Augment orbitalStrike;

	OrbitalCannon(const nlohmann::json& baseJson, float magnification);
	~OrbitalCannon() = default;

	void fire(Stage& stage) override;
	Animation* get_cannon_animation_ptr() override;
};
struct AreaCannon : public BaseCannon {
	// hits units in all lanes within a range
	float areaRange = 100;

	AreaCannon(const nlohmann::json& baseJson, float magnification);
	~AreaCannon() = default;

	void fire(Stage& stage) override;
	bool is_valid_target(Unit& target);
	inline bool within_range(float xPos) {
		float dist = (xPos - pos.x) * cannonStats.team;
		return dist <= areaRange;
	}
	Animation* get_cannon_animation_ptr() override;
};
