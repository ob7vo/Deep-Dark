#include "ActionObject.h"
#include "Stage.h"
ActionObject::ActionObject(ActionObjConfig& config) : sprite(Animation::def_texture()), 
stage(config.stage), lane(config.lane), pos(config.pos) {
}
UnitSpawner::UnitSpawner(const UnitStats* stats, std::array<Animation, 5>* aniMap, ActionObjConfig& config) :
	ActionObject(config), stats(stats), aniMap(aniMap) {
	//std::cout << "created unit spawner, pos(" << pos.x << " , " << pos.y
		//<< ")" << std::endl;
	create_animation();
}
SurgeSpawner::SurgeSpawner(const UnitStats* stats, const Augment surge, ActionObjConfig& config) :
	ActionObject(config), stats(stats), surge(surge) {
	create_animation();
}
void ActionObject::tick(float deltaTime) {
	int events = ani.update(deltaTime, sprite);

	if (Animation::check_for_event(TRIGGER, events))
		action();
	readyForRemoval = Animation::check_for_event(FINAL_FRAME, events);
}
void UnitSpawner::action() {
	if (stats->has_augment(CLONE)) stage.try_revive_unit(this);
	else {
		Unit* unit = stage.create_unit(lane, stats, aniMap);
		unit->pos.x = pos.x;
	}
}
void SurgeSpawner::action() {
	sf::Vector2f surgePos = pos;
	if (surge.augType != AugmentType::SHOCK_WAVE)
		surgePos.x += surge.value * stats->team;

	stage.create_surge(stats, lane, surge.surgeLevel, surgePos, surge.augType);
}
void UnitSpawner::create_animation() {
	std::vector<std::pair<int, AnimationEvent>> events;
	std::string path = stats->has_augment(CLONE) ? "sprites/action_objs/cloner.png" :
		"sprites/action_objs/drop_box.png";
	sf::Vector2i cellSize = { 48,40 };
	int frames = 21;
	float rate = 0.1f;
	events.emplace_back(14, TRIGGER);
	ani = Animation(path, frames, rate, cellSize, events, false);

	sprite.setTexture(ani.texture);
	ani.reset(sprite);
	sprite.setPosition(pos);
	sprite.setOrigin({ 24, 40 });
}
void SurgeSpawner::create_animation() {
	std::vector<std::pair<int, AnimationEvent>> events;
	std::string path = "sprites/action_objs/surge_spawner.png";
	sf::Vector2i cellSize = { 48,40 };
	int frames = 21;
	float rate = 0.09f;
	events.emplace_back(14, TRIGGER);
	ani = Animation(path, frames, rate, cellSize, events, false);

	sprite.setTexture(ani.texture);
	ani.reset(sprite);
}