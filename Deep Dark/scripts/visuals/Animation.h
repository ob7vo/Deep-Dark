#pragma once
#include <SFML/Graphics.hpp>
#include "json.hpp"

struct AnimationState;

const enum AnimationEvent {
	EMPTY_EVENT = 1 << 0,
	FIRST_FRAME = 1 << 1,
	FINAL_FRAME = 1 << 2,
	ATTACK = 1 << 3,
	TRIGGER = 1 << 4,
	TRIGGER_2 = 1 << 5
};

using ani_event_map = std::vector<std::pair<int, AnimationEvent>>;

struct AnimationFrame {
	sf::IntRect rect;
	float duration = 0.0f;
	int eventsMask;

	AnimationFrame(sf::IntRect rect, float duration, int events);
};
struct Animation {
	float time = 0.0f;
	int currentFrame = 0;
	bool loops = 1;
	sf::Texture texture;
	std::vector<AnimationFrame> frames;
	int frameCount = 0;

	Animation() = default;
	Animation(std::string spritePath, int frames, float framerate, 
		sf::Vector2i cellSizes, ani_event_map events, bool loops = true);

	int update(float deltaTime, sf::Sprite& sprite);
	int update(float& time, int& curFrame, float deltaTime, sf::Sprite& sprite);
	void reset(sf::Sprite& sprite);
	void reset(float& tiem, int& curFrame, sf::Sprite& sprite);

	inline int get_events() { return frames[currentFrame].eventsMask; }
	static void create_unit_animation_array(const nlohmann::json& unitFile, std::array<Animation, 5>& aniMap);
	static Animation create_unit_animation(const nlohmann::json& file, std::string ani, bool loops);
	static bool check_for_event(AnimationEvent desiredEvent, int events);


	static sf::Texture& def_texture() {
		static sf::Texture defaultTex;
		static bool initialized = false;
		if (!initialized) {
			initialized = true;
			bool shutTHeFuckUp = defaultTex.loadFromFile("sprites/defaultTexture.png");
			if (shutTHeFuckUp) return defaultTex;
		}
		return defaultTex;
	};
};