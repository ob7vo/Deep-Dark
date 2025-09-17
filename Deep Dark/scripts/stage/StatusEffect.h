#pragma once
#include "ActionStates.h"
struct StatusEffect
{
	AugmentType effect;
	float timeLeft;
	//sf::Sprite effectIcon;

	StatusEffect(AugmentType eff, float time) : effect(eff), timeLeft(time) {}
	inline void update(float deltaTime) { timeLeft -= deltaTime; }
	inline bool is_expired() const { return timeLeft < 0; }
	/*static sf::Texture& get_e_texture() {
		static sf::Texture defaultTex;
		static bool initialized = false;
		if (!initialized) {
			initialized = true;
			defaultTex.loadFromFile("sprites/defaultTexture.png");
		}
		return defaultTex;
	}*/
};
