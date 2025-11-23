#pragma once
#include "Animation.h"
#include "ProjectileData.h"
#include "Pathing.h"
#include <array>
#include <memory>

struct Lane;
class Unit;

const int ACTIVE_STATE = 0;
const int DESTROYED_STATE = 1;

struct ProjectileConfig {
	ProjectileStats stats = {};
	std::array<Animation, 2> aniArr;

	ProjectileConfig() = default;
	ProjectileConfig(int id, float magnification = 1.f);
};

class Projectile {
	sf::Vector2f pos = { 0.f, 0.f };

	float hitCountTime = 0.f;
	float maxLifespan = 999.f;
	int hitsLeft = 0;
	std::vector<std::pair<int, float>> hitUnits = {};
	ProjectileStats* stats = nullptr;

	std::array<Animation, 2>* aniArr = nullptr;
	sf::Sprite sprite = sf::Sprite(defTex);
	int frame = 0;
	float aniTime = 0.f;
	int aniState = 0;
public:
	std::unique_ptr<Pathing> pathing = nullptr;
	bool readyForRemoval = false;

	Projectile() = default;
	explicit Projectile(ProjectileConfig& config) : stats(&config.stats), aniArr(&config.aniArr){
		(*aniArr)[0].reset(aniTime, frame, sprite);
		sprite.scale({ 0.2f,0.2f });

		hitsLeft = stats->hits;
		maxLifespan = stats->maxLifespan;
	}
	~Projectile() = default;
	Projectile(Projectile&&) = default;
	Projectile& operator=(Projectile&&) = default;

	// Delete copy operations
	Projectile(const Projectile&) = delete;
	Projectile& operator=(const Projectile&) = delete;

	void tick(Lane& lane, float deltaTime);
	int update_hits_and_animation(float deltaTime);
	void attack_units(Lane& lane);
	bool within_bounds(sf::Vector2f p, float h = 1, float w = 1) const;
	bool valid_target(Unit& enemy) const;

	inline sf::Sprite& get_sprite() { return sprite; }
	inline float y_pos() const { return pos.y; }
	inline void update_hit_times(float deltaTime) {
		for (auto& [box, time] : hitUnits) time -= deltaTime;
	}
	inline void enter_destroyed_state() {
		aniState = DESTROYED_STATE;
		(*aniArr)[aniState].reset(aniTime, frame, sprite);
	}
	inline bool can_hit_again(int id) const {
		return std::find_if(hitUnits.begin(), hitUnits.end(),
			[&id](const auto& pair) {
				return pair.first == id && pair.second <= 0.f;
			}) == hitUnits.end();
	}
};
