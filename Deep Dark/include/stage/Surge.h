#pragma once
#include "Animation.h"
#include "UnitEnums.h"
#include "UnitData.h"
#include <iostream>
#include <memory>

enum class SurgeAnimationStates {
	START_UP = 0,
	ACTIVE = 1,
	ENDING = 2
};

const float SW_WIDTH = 25.f;
const float SW_BASE_DISTANCE = 75.f;
const float SW_DISTANCE_PER_LEVEL = 50.f;
const float SW_TWEEN_TIMER = .75f;
const float SW_TWEEN_TIME_PER_LEVEL = .50f;

const float FW_WIDTH = 30.f;
const float FW_TIMER = .66f;

const float OS_WIDTH = 40.f;

class Unit;
struct Stage;
struct Lane;
struct UnitTween;

struct Surge
{
	bool readyForRemoval = 0;
	bool createdByCannon = false;

	float width = 10.f;
	sf::Vector2f pos;
	int currentLane = 0;

	AugmentType surgeType;
	const UnitStats* stats;
	int hitIndex;

	float aniTime = 0.0f;
	int currentFrame = 0;
	SurgeAnimationStates animationState;
	sf::Sprite sprite;

	std::vector<int> hitUnits;

	Surge(const UnitStats* stats, int curLane, sf::Vector2f pos, AugmentType surgeType);
	virtual ~Surge() = default;
	sf::Texture& get_base_texture(AugmentType surgeType);

	void attack_units(Lane& enemyUnits);
	void on_kill(Unit& enemyUnit);
	bool try_terminate_unit(Unit& enemyUnit, int dmg = 0);
	virtual void tick(float deltaTime, Stage& stage) = 0;
	virtual int update_animation(float deltaTime) = 0;
	virtual void start_animation(SurgeAnimationStates newState) {};
	virtual bool never_hit_unit(int id) { return !already_hit_unit(id); }

	inline bool already_hit_unit(int id) { 
		return std::find(hitUnits.begin(), hitUnits.end(), id) != hitUnits.end(); 
	}
	inline bool immune_to_surge_type(size_t unitImmunities) const { return surgeType & unitImmunities; }
	inline bool targeted_by_unit(int enemyTargetTypes) const { return stats->targeted_by_unit(enemyTargetTypes); }
	inline int get_dmg() { return stats->get_hit_stats(hitIndex).dmg; }

	static void init_animations();
};
struct ShockWave : public Surge {
	std::unique_ptr<UnitTween> tween = nullptr;
	int id;
	ShockWave(const UnitStats* stats, int curLane, int level, sf::Vector2f pos, Stage& stage);
	~ShockWave() = default;

	void tick(float deltaTime, Stage& stage) override;
	int update_animation(float deltaTime) override;
	void start_animation(SurgeAnimationStates newState) override;
	inline bool tweening() const { return tween != nullptr; }
	inline void cancel_tween() { tween = nullptr; }
};
struct FireWall : public Surge {
	std::vector<int> permanentHitUnits;
	float timeLeft;
	int level;

	FireWall(const UnitStats* stats, int curLane, int level, sf::Vector2f pos);
	~FireWall() = default;

	void tick(float deltaTime, Stage& stage) override;
	int update_animation(float deltaTime) override;
	void start_animation(SurgeAnimationStates newState) override;
	bool never_hit_unit(int id) override;
};
struct OrbitalStrike : public Surge {
	OrbitalStrike(const UnitStats* stats, int curLane, sf::Vector2f pos);
	~OrbitalStrike() = default;

	void tick(float deltaTime, Stage& stage) override;
	int update_animation(float deltaTime) override;
};

