#pragma once
#include "BaseCannon.h"
#include "Animation.h"
#include <memory>

struct Stage;
const sf::Texture defBaseTexture("sprites/defaults/defaultTexture.png");

class Base
{
private:
	sf::Sprite sprite = sf::Sprite(defBaseTexture);
	int team = 0;
	int maxHp = 1;
	int hp = 1;

	float cooldown = 0;
	float cannonTimer = 0;
	float sightRange = 0; // only for enemy bases

public:
	bool tookDmgThisFrame = false; // this variable doesnt matter to player base

	std::unique_ptr<BaseCannon> cannon;
	Animation* cannonAnimation = nullptr;
	sf::Vector2f pos = {0.f, 0.f};

	Base() = default;
	Base(const nlohmann::json& stageFile, int team);
	void create_cannon(std::string path, float mag);

	void take_damage(int dmg);
	bool try_fire_cannon();
	void tick(Stage& stage, float deltaTime);

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
			bool shutTHeFuckUp = defaultTex.loadFromFile("sprites/defaults/defaultTexture.png");
			if (shutTHeFuckUp) return defaultTex;
		}
		return defaultTex;
	}
	inline void draw(sf::RenderWindow& window) { window.draw(sprite); }
	inline float get_hp_percentage() {
		if (hp <= 0 || maxHp <= 0) return 0.f;
		else return (float)hp / maxHp;
	}
	inline bool on_cooldown() const { return cooldown > 0.f; }
};

