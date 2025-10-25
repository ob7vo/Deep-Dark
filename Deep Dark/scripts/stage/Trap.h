#pragma once
#include "Animation.h"
#include "UnitEnums.h"
const float TRAP_HEIGHT = 15.0f;
enum class TrapType {
	NONE = -1,
	LAUNCH_PAD = 0,
	TRAP_DOOR,
	FLAT_DMG,
	PERCENT_DMG,
	EFFECT
};

struct Lane;
class Unit;
struct Stage;
struct StageRecord;

struct Trap
{
	Lane& lane;
	std::pair<float, float> triggerRange;
	std::pair<float, float> attackRange;
	sf::Vector2f pos = { 0.f, 0.f };

	float timeLeft = 0.0f;
	float checkTimer = 0.0f;

	sf::Sprite sprite;
	Animation ani;
	bool animating = false;

	TrapType trapType = TrapType::NONE;
	bool triggered = false;
	float dmgValue = 0;
	Augment aug = {};

	Trap(Lane& lane, const nlohmann::json& json);
	virtual ~Trap() = default;

	bool enemy_in_trigger_range();
	void tick(float deltaTime, StageRecord& rec);
	void trigger(StageRecord& rec);
	void trigger_launch_pad();
	void trigger_trap_door();
	void trigger_attack();
	bool valid_attack_target(Unit& unit);
	bool in_trigger_range(Unit& unit);

	static Animation get_trap_animation(TrapType type);
};