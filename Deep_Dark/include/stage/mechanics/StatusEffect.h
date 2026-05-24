#pragma once
#include "AugmentTypes.h"

struct StatusEffect
{
	AugmentType effect;
	float value;
	float timeLeft;

	StatusEffect(AugmentType eff, float val, float time) :
		effect(eff), value(val), timeLeft(time) {}
	inline void update(float deltaTime) { timeLeft -= deltaTime; }
	inline bool is_expired() const { return timeLeft < 0; }
};
