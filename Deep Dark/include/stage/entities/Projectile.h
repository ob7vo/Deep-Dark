#pragma once
#include "Animation.h"
#include "ProjectileData.h"
#include "Pathing.h"
#include "StageEntity.h"

struct Stage;
struct UnitPool;
struct Lane;
class Unit;

const int ACTIVE_STATE = 0;
const int DESTROYED_STATE = 1;

struct ProjectileConfig {
	ProjectileStats stats = {};
	std::array<sf::Texture, 2> textures;
	std::array<AnimationClip, 2> aniArr;

	ProjectileConfig() = default;
	explicit ProjectileConfig(int id, float magnification = 1.f);
};

class Projectile : public StageEntity {
private:
	float maxLifespan = 999.f;

	float hitCountTime = 0.f;
	int hitsLeft = 0;
	std::vector<std::pair<int, float>> hitUnits = {};

	ProjectileStats* stats = nullptr;

	std::array<AnimationClip, 2>* aniArr = nullptr;
	int aniState = 0;

public:

	std::unique_ptr<Pathing> pathing = nullptr;

	Projectile() = default;
	explicit Projectile(ProjectileConfig& config);
	~Projectile() = default;
	Projectile(Projectile&&) = default;
	Projectile& operator=(Projectile&&) = default;

	// Delete copy operations
	Projectile(const Projectile&) = delete;
	Projectile& operator=(const Projectile&) = delete;

	void tick(Stage& stage, float deltaTime);
	void draw(sf::RenderWindow& window) const;

	AnimationEvent update_hits_and_animation(float deltaTime);
	void attack_units(Stage& stage);
	bool within_bounds(sf::Vector2f p) const;
	bool valid_target(const Unit& enemy) const;

	inline sf::Sprite& get_sprite() { return sprite; }
	inline float y_pos() const { return pos.y; }
	void update_hit_times(float deltaTime);
	void enter_destroyed_state();
	bool can_unit_hit_again(int id) const;
};
