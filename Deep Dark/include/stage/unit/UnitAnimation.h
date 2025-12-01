#pragma once
#include "Animation.h"
#include "SFML/Graphics/RectangleShape.hpp"

class Unit;

struct UnitAnimation {
private:
	UnitAnimationState state = UnitAnimationState::MOVE;

	float time = 0.f;
	int currentFrame = 0;
	UnitAniMap* aniMap; // Owned by Enemy Spawners and Loadout slots
	sf::Sprite sprite = sf::Sprite(defTex);

	sf::RectangleShape marker;
public:
	explicit UnitAnimation(UnitAniMap* map);

	void start(UnitAnimationState newState);
	void draw(sf::RenderWindow& window) const;
	void start_idle_or_attack_animation(Unit& unit);
	int update(float deltaTime);
	void set_position(sf::Vector2f pos);

	inline bool falling() const { return state == UnitAnimationState::FALLING; }
	inline bool dying() const { return state == UnitAnimationState::DYING; }
	inline bool in_knockback() const { return state == UnitAnimationState::KNOCKBACK; }
	inline bool is_phasing() const { return state == UnitAnimationState::IS_PHASING; }
	inline bool invincible() const { return static_cast<int>(state) > 2; }

	inline void die() { state = UnitAnimationState::DYING; }
	inline void start_phasing() { state = UnitAnimationState::IS_PHASING; }
	inline UnitAniMap* get_ani_map() const { return aniMap; }
	inline UnitAnimationState get_state() const { return state; }
};