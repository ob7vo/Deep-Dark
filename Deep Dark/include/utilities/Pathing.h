#pragma once
#include "SFML/System/Vector2.hpp"
#include <Easing.hpp>
#include <iostream>
#include <vector>

enum class PathingType {
	CIRCULAR,
	PROJECTILE,
	WAYPOINTS
};

struct Pathing {
	sf::Vector2f pos = { 0.f,0.f };
	union {
		float speed = 0;
		float timer;
	};

	Pathing(sf::Vector2f pos, float spd) : speed(spd), pos(pos) {}
	virtual ~Pathing() = default;

	virtual sf::Vector2f move(float deltaTime) = 0;
	sf::Vector2f get_pos() const { return pos; }

	virtual PathingType get_type() const = 0;
	virtual bool moving_down() = 0;
	static PathingType string_to_type(std::string& str);
};
struct CirclePathing : public Pathing {
	sf::Vector2f center = { 0.f,0.f };
	float radius = 0;
	float angleDeg = 0;

	CirclePathing(sf::Vector2f cen, float spd, float rad, float angle) : Pathing(cen, spd),
		center(cen), radius(), angleDeg(angle) {
		move(0.02f);
	}
	~CirclePathing() = default;

	sf::Vector2f move(float deltaTime) override;
	PathingType get_type() const override { return PathingType::CIRCULAR; }
	bool moving_down() override { return true; }
};
struct ProjectilePathing : public Pathing {
	sf::Vector2f velocity = { 0.f,0.f };
	float mass = 0; // acts as gravity

	ProjectilePathing(sf::Vector2f pos, sf::Vector2f vel, float m) : 
		Pathing(pos,0.f), velocity(vel), mass(m){}
	~ProjectilePathing() = default;

	sf::Vector2f move(float deltaTime) override;
	PathingType get_type() const override { return PathingType::PROJECTILE; }
	bool moving_down() { return velocity.y < 0; }
};
struct WaypointPathing : public Pathing {
	sf::Vector2f start = { 0.f,0.f };
	int curWp = 1;
	int loop = 0;
	int maxLoops = 1;
	std::vector<sf::Vector2f> waypoints = {};

	float timeElapsed = 0;
	EasingType easeType = EasingType::LINEAR;

	WaypointPathing(std::vector<sf::Vector2f> wps, int loops, 
		float timer, EasingType ease) :Pathing(wps[0], timer),
		start(wps[0]), waypoints(wps), easeType(ease), maxLoops(loops) { }
	~WaypointPathing() = default;

	sf::Vector2f move(float deltaTime) override;
	PathingType get_type() const override { return PathingType::WAYPOINTS; }
	bool moving_down() override { return (waypoints[curWp] - start).y < 0; }
};
