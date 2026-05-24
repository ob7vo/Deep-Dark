#pragma once
#include "UnitStats.h"
#include "Animation.h"
#include <json_fwd.hpp>
#include <SFML/System/Vector2.hpp>

struct Stage;
struct BaseStats;
struct Animation;
class Unit;
class Base;

struct BaseCannon
{
	sf::Vector2f pos = {};
	UnitStats cannonStats;
	int team = 1;

	AnimationClip animClip;

	BaseCannon(const nlohmann::json& baseJson, float magnification);
	virtual ~BaseCannon() = default;

	virtual void fire(Stage& stage) = 0;
	virtual void create_animation() = 0;
};
struct WaveCannon : public BaseCannon {
public:
	Augment shockWave;

	WaveCannon(const nlohmann::json& baseJson, float magnification);
	~WaveCannon() override  = default;

	void fire(Stage& stage) override;
	void create_animation() override;
};
struct FireWallCannon : public BaseCannon {
	Augment fireWall;

	FireWallCannon(const nlohmann::json& baseJson, float magnification);
	~FireWallCannon() override = default;

	void fire(Stage& stage) override;
	void create_animation() override;
};
struct OrbitalCannon : public BaseCannon {
	//spawns 4 equally spaced orbital strikes
	Augment orbitalStrike;

	OrbitalCannon(const nlohmann::json& baseJson, float magnification);
	~OrbitalCannon() override = default;

	void fire(Stage& stage) override;
	void create_animation() override;
};
struct AreaCannon : public BaseCannon {
	// hits units in all lanes within a range
	float areaRange = 100;

	AreaCannon(const nlohmann::json& baseJson, float magnification);
	~AreaCannon() override = default;

	void fire(Stage& stage) override;
	bool is_valid_target(const Unit& target) const;
	inline bool within_range(float xPos) const {
		float dist = (xPos - pos.x) * static_cast<float>(cannonStats.team);
		return dist <= areaRange;
	}

	void create_animation() override;
};
