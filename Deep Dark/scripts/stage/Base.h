#pragma once
#include "BaseCannon.h"
#include "Animation.h"
#include <memory>

struct Stage;

class Base
{
private:
	sf::Sprite sprite;
	int team = 0;
	int maxHp = 0;
	int hp = 0;

	float cooldown = 0;
	float cannonTimer = 0;
	float sightRange = 0; // only for enemy bases

public:
	std::unique_ptr<BaseCannon> cannon;
	Animation* cannonAnimation = nullptr;
	sf::Vector2f pos = {0.f, 0.f};

	Base(const nlohmann::json& stageFile, int team);
	void create_cannon(std::string path, float mag);

	void take_damage(int dmg);
	bool try_fire_cannon();
	void tick(Stage& stage, sf::RenderWindow& window, float deltaTime);

	//void get_animation_ptr(std::string cannonType);
	inline bool enemy_in_range(float xPos) {
		float dist = (xPos - pos.x) * team;
		return dist >= 0 && dist <= sightRange;
	}
	static sf::Texture& get_default_texture() {
		static sf::Texture defaultTex;
		static bool initialized = false;
		if (!initialized) {
			initialized = true;
			bool shutTHeFuckUp = defaultTex.loadFromFile("sprites/defaultTexture.png");
			if (shutTHeFuckUp) return defaultTex;
		}
		return defaultTex;
	}
	inline void draw(sf::RenderWindow& window) { window.draw(sprite); }
	inline bool on_cooldown() const { return cooldown > 0.f; }
};

