#pragma once
#include "TextureManager.h"
#include "UnitEnums.h"

struct AnimationState;

const enum AnimationEvent {
	EMPTY_EVENT = 0,
	FIRST_FRAME = 1 << 0,
	FINAL_FRAME = 1 << 1,
	ATTACK = 1 << 2,
	TRIGGER = 1 << 3,
	TRIGGER_2 = 1 << 4
};

struct Animation;

using UnitAniMap = std::unordered_map<UnitAnimationState, Animation>;
using AnimationEventsList = std::vector<std::pair<int, int>>;

struct AnimationFrame {
	sf::IntRect rect;
	float duration = 0.0f;
	int eventsMask;

	AnimationFrame(sf::IntRect rect, float duration, int events = 0);
};
struct Animation {
	float timeElapsed = 0.0f;
	bool loops = true;
	sf::Texture texture;
	sf::Vector2f origin;

	int currentFrame = 0;
	std::vector<AnimationFrame> frames;
	int frameCount = 0;

	Animation() = default;
	Animation(const std::string_view& spritePath, int frames, float framerate, 
		sf::Vector2i cellSizes, sf::Vector2f origin, AnimationEventsList eventList = {}, bool loops = false);

	int update(float deltaTime, sf::Sprite& sprite);
	int update(float& time, int& curFrame, float deltaTime, sf::Sprite& sprite);
	
	/// <summary>
	/// Starts the animation. Sets the current frame and time to 0, 
	/// then sets the Sprite's Texture, Rect, and Origin
	/// </summmary>
	void reset(sf::Sprite& sprite);
	/// <summary>
	/// Starts the animation. Sets the current frame and time to 0, 
	/// then sets the Sprite's Texture, Rect, and Origin
	/// </summmary> 
	void reset(float& tiem, int& curFrame, sf::Sprite& sprite);

	inline int get_events() { return frames[currentFrame].eventsMask; }
	static void setup_unit_animation_map(const nlohmann::json& unitFile, UnitAniMap& aniMap);
	static Animation create_unit_animation(const nlohmann::json& file, const std::string_view& ani, const std::string_view& path, bool loops);
	static bool check_for_event(AnimationEvent desiredEvent, int events);

};