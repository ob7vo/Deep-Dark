#pragma once
#include "Animation.h"
#include "UnitEnums.h"

struct UnitStats;
struct Stage;

struct ActionObjConfig {
	Stage& stage;
	int lane = 0;
	sf::Vector2f pos;

	ActionObjConfig(Stage& s, int l, sf::Vector2f p) :
		stage(s), lane(l), pos(p) { }
};
struct ActionObject
{
	Stage& stage;
	bool readyForRemoval = false;

	sf::Vector2f pos = { 0.f, 0.f };
	int lane = 0;

	Animation ani;
	sf::Sprite sprite;

	ActionObject(ActionObjConfig& config);
	~ActionObject() = default;

	void tick(sf::RenderWindow& window, float deltaTime);
	virtual void action() = 0;
	virtual void create_animation() = 0;
};
struct UnitSpawner : public ActionObject {
	const UnitStats* stats;
	std::array<Animation, 5>* aniMap;

	UnitSpawner(const UnitStats* stats, std::array<Animation, 5>* aniMap, ActionObjConfig& config);
	void action() override;
	void create_animation() override;
};
struct SurgeSpawner : public ActionObject {
	const UnitStats* stats;
	const Augment surge;

	SurgeSpawner(const UnitStats* stats, const Augment surge, ActionObjConfig& config);
	void action() override;
	void create_animation() override;
};

