#pragma once
#include <Easing.hpp>
#include <SFML/System/Vector2.hpp>

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

	Pathing(sf::Vector2f pos, float spd) : pos(pos), speed(spd) {}
	virtual ~Pathing() = default;

	virtual sf::Vector2f move(float deltaTime) = 0;
	sf::Vector2f get_pos() const { return pos; }

	virtual PathingType get_type() const = 0;
	virtual bool moving_down() const = 0;

	inline virtual sf::Vector2f getStartingPosition() const = 0;

	static PathingType string_to_type(std::string& str);
};
struct CirclePathing : public Pathing {
	sf::Vector2f center = { 0.f,0.f };
	float radius = 0;
	float angleDeg = 0;

	CirclePathing(sf::Vector2f cen, float spd, float rad, float angle) : Pathing(cen, spd),
		center(cen), radius(rad), angleDeg(angle) {
		pos = { cen.x + rad, cen.y + rad };
	}
	~CirclePathing() override = default;

	sf::Vector2f move(float deltaTime) override;

	PathingType get_type() const override { return PathingType::CIRCULAR; }
	bool moving_down() const override { return true; }

	inline sf::Vector2f getStartingPosition() const override { return { center.x + radius, center.y + radius }; }
};
struct ProjectilePathing : public Pathing {
	sf::Vector2f velocity = { 0.f,0.f };
	float mass = 0; // acts as gravity

	ProjectilePathing(sf::Vector2f pos, sf::Vector2f vel, float m) : 
		Pathing(pos,0.f), velocity(vel), mass(m){}
	~ProjectilePathing() override = default;

	sf::Vector2f move(float deltaTime) override;

	PathingType get_type() const override { return PathingType::PROJECTILE; }
	bool moving_down() const override { return velocity.y < 0; }

	// Doesnt really work for this
	inline sf::Vector2f getStartingPosition() const override { return pos; }
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
		start(wps[0]), maxLoops(loops), waypoints(wps), easeType(ease){ }
	~WaypointPathing() override = default;

	sf::Vector2f move(float deltaTime) override;

	PathingType get_type() const override { return PathingType::WAYPOINTS; }
	bool moving_down() const override { return (waypoints[curWp] - start).y < 0; }

	inline sf::Vector2f getStartingPosition() const override { return start; }
};
