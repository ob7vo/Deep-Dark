#pragma once
#include "UnitEnums.h"
struct StatusEffect
{
	AugmentType effect;
	float timeLeft;

	StatusEffect(AugmentType eff, float time) : effect(eff), timeLeft(time) {}
	inline void update(float deltaTime) { timeLeft -= deltaTime; }
	inline bool is_expired() const { return timeLeft < 0; }
};
