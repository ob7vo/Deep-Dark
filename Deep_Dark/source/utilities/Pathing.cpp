#include "pch.h"
#include "Pathing.h"
#include "Easing.hpp"

const float HALF_CIRC = 180.f;
const float THRESHOLD_SQ = 4.0f; // e.g. 2 units tolerance squared

PathingType Pathing::string_to_type(std::string& str) {
	switch (str[0]) {
	case 'c': return PathingType::CIRCULAR;
	case 'p': return PathingType::PROJECTILE;
	case 'w': return PathingType::WAYPOINTS;
	default: 
		std::cout << "cannot convert str: [" << str << "] to a PathingType" << std::endl;
		return PathingType::CIRCULAR;
	}

	return PathingType::CIRCULAR;
}
sf::Vector2f CirclePathing::move(float deltaTime) {
	float angleRad = angleDeg * (PI / 180.f);

	pos.x = center.x + radius * cos(angleRad);
	pos.y = center.y + radius * sin(angleRad);

	angleDeg += speed * deltaTime;

	return pos;
}
sf::Vector2f ProjectilePathing::move(float dt) {
	// If this is gravity, there should either be a constant G value
	// or "mass" should be renamed to some gravity variable
	// since mass is canceled out when turning Fg into acceleration
	velocity.y -= mass * dt; 

	pos.x += velocity.x * dt;
	pos.y -= velocity.y * dt;

	return pos;
}
sf::Vector2f WaypointPathing::move(float dt) {
	timeElapsed += dt;
	float t = timeElapsed / timer;
	t = easeFuncArr[(int)easeType](t);

	sf::Vector2f wp = waypoints[curWp];
	pos = start + (wp - start) * t;

	sf::Vector2f delta = wp - pos;
	float distSq = delta.x * delta.x + delta.y * delta.y;
	if (distSq < THRESHOLD_SQ) {
		curWp++;
		start = wp;
		timeElapsed = 0.f;

		if (curWp >= (int)waypoints.size()) {
			curWp = 0;
			loop++;
		}
	}

	return pos;
}