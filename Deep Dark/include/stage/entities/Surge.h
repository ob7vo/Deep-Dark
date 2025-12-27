#pragma once
#include "AugmentTypes.h"
#include "UnitTween.h"
#include "StageEntity.h"

struct UnitStats;
namespace sf { class RenderWindow; };

enum class SurgeAnimationStates {
	START_UP = 0,
	ACTIVE = 1,
	ENDING = 2
};

class Unit;
struct Stage;
struct Lane;

struct Surge : public StageEntity
{
	bool createdByCannon = false;

	float halfWidth = 10.f;

	AugmentType surgeType;
	const UnitStats* stats;
	// The # hit when the surge was spawn by the Unit
	int hitIndex = -1;

	SurgeAnimationStates animationState;
	sf::RectangleShape hitbox = sf::RectangleShape({32.f,32.f});

	std::vector<int> hitUnits;

	Surge(const UnitStats* stats, int curLane, sf::Vector2f pos);
	~Surge() override = default;

	void attack_units(Lane& enemyUnits);
	void on_kill(Unit& enemyUnit) const;
	int calculate_damage_and_effects(Unit& unit) const;
	bool try_terminate_unit(const Unit& enemyUnit, int dmg = 0) const;
		
	bool valid_target(const Unit& unit) const;

	AnimationEvent update_animation(Stage& stage, float deltaTime) override;
	virtual void start_animation(SurgeAnimationStates newState) {};
	void draw(sf::RenderWindow& window) const;

	/// <summary> Sets the sur'ges htiIndex to 0 and sets it as createdByCannon </summary>
	inline void set_as_cannon_creation() { createdByCannon = true; hitIndex = 0; }

	/// <summary>
	/// This is need so that Counter Surge wont activate multiple times
	/// for FireWalls, since thos hit Units more than once
	/// </summary>
	virtual bool never_hit_unit(int id) { return !already_hit_unit(id); }
	bool already_hit_unit(int id) const;

	bool targeted_by_unit(UnitType enemyTargetTypes) const;
	bool in_range(float x) const;
	int get_dmg() const;

	virtual AugmentType surge_type() const = 0;
	static void init_animations();
};

struct ShockWave : public Surge {
	UnitTween tween;
	int id;

	ShockWave(const UnitStats* stats, int curLane, int level, sf::Vector2f pos, Stage& stage);
	~ShockWave() final = default;

	void tick(Stage& stage, float deltaTime) override;
	AnimationEvent update_animation(Stage& stage, float deltaTime) final;
	void start_animation(SurgeAnimationStates newState) override;
	inline bool tweening() const { return tween.active; }
	inline void cancel_tween() { tween.active = false; }

	inline AugmentType surge_type() const override { return AugmentType::SHOCK_WAVE; }
	static std::array<AnimationClip, 2> shockWaveAni;
};
struct FireWall : public Surge {
	std::vector<int> permanentHitUnits;
	float timeLeft;
	int level;

	FireWall(const UnitStats* stats, int curLane, int level, sf::Vector2f pos);
	~FireWall() final = default;

	void tick(Stage& stage, float deltaTime) override;
	void start_animation(SurgeAnimationStates newState) override;
	bool never_hit_unit(int id) override;

	inline AugmentType surge_type() const override { return AugmentType::FIRE_WALL; }
	static std::array<AnimationClip, 3> fireWallAni;
};
struct OrbitalStrike : public Surge {
	OrbitalStrike(const UnitStats* stats, int curLane, sf::Vector2f pos);
	~OrbitalStrike() final = default;

	void tick(Stage& stage, float deltaTime) override;

	inline AugmentType surge_type() const override { return AugmentType::ORBITAL_STRIKE; }
	static AnimationClip orbitalStrikeAni;
};

