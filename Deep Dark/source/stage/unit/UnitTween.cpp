#include "pch.h"
#include "Easing.hpp"
#include "UnitTween.h"

void UnitTween::start(sf::Vector2f start, const sf::Vector2f& end, float dur, UnitMoveRequestType type) {
	startPos = start;
	endPos = end;
	duration = dur;
	tweenType = type;

	elapsedTime = 0.f;
	active = true;
}
void UnitTween::update(sf::Vector2f& unitPos, float deltaTime) {
	if (!active) return;
	elapsedTime += deltaTime;

	if (elapsedTime >= duration) {
		//std::cout << "finished tweening" << std::endl;

		elapsedTime = duration;
		updateValue(unitPos);
		active = false;
	}
	else updateValue(unitPos);
}
sf::Vector2f UnitTween::getEasedT() const {
	float t = elapsedTime / duration;
	const auto& [easeX, easeY] = unitTweenEasings[static_cast<int>(tweenType)];

	float tX = easeFuncArr[static_cast<int>(easeX)](t);
	float tY = easeFuncArr[static_cast<int>(easeY)](t);

	return { tX, tY };
}
