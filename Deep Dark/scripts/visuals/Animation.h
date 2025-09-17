#pragma once
#include <SFML/Graphics.hpp>
#include "json.hpp"
struct AnimationState;

enum class AnimationEvent {
	NONE,
	FIRST_FRAME,
	FINAL_FRAME,
	UNIT_ATTACK,
	BASE_FIRE
};
struct AnimationFrame {
	sf::IntRect rect;
	float duration = 0.0f;
	std::vector<AnimationEvent> events;

	AnimationFrame(sf::IntRect rect, float duration, std::vector<AnimationEvent> events);
};
struct Animation {
	float time = 0.0f;
	int currentFrame = 0;
	bool loops = 1;
	sf::Texture texture;
	std::vector<AnimationFrame> frames;
	int frameCount = 0;

	Animation() = default;
	Animation(std::string spritePath, int frames, float framerate, int textureSizes[2], int cellSizes[2], std::vector<std::pair<int, AnimationEvent>> events, bool loops);

	std::vector<AnimationEvent> update(float deltaTime, sf::Sprite& sprite);
	std::vector<AnimationEvent> update(float& time, int& curFrame, float deltaTime, sf::Sprite& sprite);
	void start(sf::Sprite& sprite);
	void start(float& tiem, int& curFrame, sf::Sprite& sprite);

	static void create_unit_animation_array(const nlohmann::json& unitFile, std::array<Animation, 5>& aniMap);
	static Animation create_unit_animation(const nlohmann::json& file, std::string ani, bool loops);
	static bool check_for_event(AnimationEvent desiredEvent, std::vector<AnimationEvent> events);
};
struct AnimationState {
	Animation* ani;
	float time;
	int currentFrame;

	inline void update(float deltaTime, sf::Sprite& sprite) {
		ani->update(time, currentFrame, deltaTime, sprite);
	}
	inline void start(sf::Sprite sprite) {
		ani->start(time, currentFrame, sprite);
	}
};
