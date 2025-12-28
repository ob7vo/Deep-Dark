#pragma once
#include "TextureManager.h"
#include <SFML\Graphics\Sprite.hpp>
#include "UnitEnums.h"
#include "AnimationEvent.h"
#include <deque>
#include <unordered_map>

struct AnimationClip;

using UnitAniMap = std::unordered_map<UnitAnimationState, AnimationClip>;

struct AnimationFrame {
	sf::IntRect rect;
	float duration = 0.0f;
	AnimationEvent eventsMask;

	AnimationFrame(sf::IntRect rect, float duration, AnimationEvent events = AnimationEvent::EMPTY_EVENT);
};
struct AnimationClip {
	sf::Texture* texture = nullptr;   // NOT owning
	sf::Vector2f origin = { 0.f, 0.f };
	std::vector<AnimationFrame> frames = {};
	bool loops = true;

	AnimationClip() = default;
	AnimationClip(sf::Texture* pTexture, int frames, float framerate,
		sf::Vector2i cellSizes, sf::Vector2f origin, const AnimationEventsList& eventList = {},
		bool loops = false);

	void set_duration(float duration);

	/// <summary>
	/// Takes passed in aniMap and unitTexture vector and sets them up.
	/// This function EXPECTS the two variables to be EMPTY when called
	/// </summary>
	static void setup_unit_animation_map(const nlohmann::json& unitFile, UnitAniMap& aniMap, std::deque<sf::Texture>& unitTextures);
	static AnimationClip create_unit_animation(const nlohmann::json& file, sf::Texture* pTexture, 
		UnitAnimationState ani, bool loops);
	static AnimationClip from_json(const nlohmann::json& file, sf::Texture* pTexture, bool loops);
};
struct AnimationPlayer {
	const AnimationClip* clip = nullptr;

	float time = 0.f;
	int currentFrame = 0;

	AnimationPlayer() = default;

	AnimationEvent update(float deltaTime, sf::Sprite& sprite);
	/// <summary>  Sets the new clip then calls reset </summmary>
	void start(const AnimationClip* newClip, sf::Sprite& sprite);
	/// <summary>
	/// Sets the Sprites origin, texture, and rect,then resets
	/// currentFrame and time to 0.
	/// </summary>
	void reset(sf::Sprite& sprite);
	
	// Sets only the TIME and CURFFRAME to 0
	inline void restart() { time = currentFrame = 0; }
	inline AnimationEvent get_events() const { return clip->frames[currentFrame].eventsMask; }
	inline bool onFirstFrame() const { return currentFrame == 0; }
	inline bool onFinalFrame() const { return currentFrame == clip->frames.size() - 1; }
};