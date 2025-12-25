#pragma once
#include "Animation.h"
#include "SFML/Graphics/RectangleShape.hpp"

class Unit;
struct UnitStats;

struct UnitAnimation {
private:
	UnitAnimationState state = UnitAnimationState::MOVE;

	AnimationPlayer player;
	UnitAniMap* aniMap; // Owned by Enemy Spawners and Loadout slots

	sf::Sprite sprite = sf::Sprite(defTex);
	sf::RectangleShape hurtbox;
public:
	bool drawable = true;

	UnitAnimation(UnitAniMap* map, const UnitStats* stats);
	~UnitAnimation() = default;

	void start(UnitAnimationState newState);
	void draw(sf::RenderWindow& window) const;
	void start_idle_or_attack_animation(Unit& unit);
	void start_move_idle_or_attack(Unit& unit);
	void set_position(sf::Vector2f pos);

	inline AnimationEvent update(float deltaTime) {
		return player.update(deltaTime, sprite);
	}

	void update_visual_state();
	void enter_is_phasing_state();

	inline bool falling() const { return state == UnitAnimationState::FALLING; }
	inline bool dying() const { return state == UnitAnimationState::DEATH; }
	inline bool dead() const { return dying() && player.onFinalFrame(); }
	inline bool in_knockback() const { return state == UnitAnimationState::KNOCKBACK; }
	inline bool is_phasing() const { return state == UnitAnimationState::PHASE_ACTIVE; }
	inline bool invincible() const { return static_cast<int>(state) > 2; }
	inline bool onFirstFrame() const { return player.currentFrame == 0; }

	inline void die() { state = UnitAnimationState::DEATH; }
	inline UnitAniMap* get_ani_map() const { return aniMap; }
	inline const sf::Sprite& get_sprite() const { return sprite; }
	inline UnitAnimationState get_state() const { return state; }
};