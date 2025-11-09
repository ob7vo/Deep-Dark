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

	int lane = 0;

	Animation ani;
	sf::Sprite sprite = sf::Sprite(defTex);

	ActionObject(ActionObjConfig& config);
	~ActionObject() = default;

	void tick(float deltaTime);
	virtual void action() = 0;
	virtual void create_animation() = 0;
};
struct UnitSpawner : public ActionObject {
	const UnitStats* stats;
	UnitAniMap* aniMap;

	UnitSpawner(const UnitStats* stats, UnitAniMap* aniMap, ActionObjConfig& config);
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

