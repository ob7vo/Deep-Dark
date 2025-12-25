#pragma once
#include "StageEntity.h"
#include "Augment.h"

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

struct Trap : public StageEntity
{
private:
	std::pair<float, float> triggerRange;
	std::pair<float, float> attackRange;

	float timeLeft = 0.0f;
	float checkTimer = 0.0f;

	TrapType trapType = TrapType::NONE;
	bool triggered = false;
	float dmgValue = 0;
	Augment aug = {};

	AnimationClip animClip;
public:

	Trap(const nlohmann::json& json, sf::Vector2f pos, int lane);
	~Trap() override = default;

	void tick(Stage& stage, float deltaTime) override;
	void action(Stage& stage) override;
	AnimationEvent update_animation(Stage& stage, float deltaTime) override;

	bool enemy_in_trigger_range(Stage& stage) const;
	void trigger_launch_pad(Stage& stage) const; 
	void trigger_trap_door(Stage& stage);

	void trigger_attack(Stage& stage) const;
	void attack_lane(std::vector<Unit>& units) const;
	bool valid_attack_target(const Unit& unit) const;
	bool in_trigger_range(const Unit& unit) const;

	static AnimationClip create_trap_animation(TrapType type);
};