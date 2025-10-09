#include "Surge.h"
#include "Tween.h"
#include "Stage.h"
#include "Unit.h"

static std::array<Animation, 2> shockWaveAni;
static std::array<Animation, 3> fireWallAni;
static Animation orbitalStrikeAni;

sf::Texture& Surge::get_base_texture(AugmentType surgeType) {
	static sf::Texture texture;
	(void)texture.loadFromFile("sprites/defaultTexture.png");
	return texture;
}

Surge::Surge(const UnitStats* stats, int curLane, sf::Vector2f pos, AugmentType surgeType) :
	stats(stats), currentLane(curLane), pos(pos), surgeType(surgeType),
	sprite(get_base_texture(surgeType)) 
{
	hitUnits.reserve(12);
	sprite.setPosition(pos);
}
ShockWave::ShockWave(const UnitStats* stats, int curLane, int level, sf::Vector2f pos, Stage& stage) :
	Surge(stats, curLane, pos, SHOCK_WAVE), id(stage.nextUnitID++){
	width = SW_WIDTH;

	sf::Vector2f endPos = pos;
	endPos.x += (SW_BASE_DISTANCE + (SW_DISTANCE_PER_LEVEL * level - 1)) * stats->team;
	float timeLeft = SW_TWEEN_TIMER + (SW_TWEEN_TIME_PER_LEVEL * level - 1);

	UnitTween* tween = stage.create_tween(id, pos, endPos, timeLeft,
		RequestType::NONE);
	if (tween) tween->easingFuncX = EasingType::LINEAR;

	sprite.setOrigin({ 16, 32 });
	animationState = SurgeAnimationStates::ACTIVE;
	shockWaveAni[0].reset(aniTime, currentFrame, sprite);
}
FireWall::FireWall(const UnitStats* stats, int curLane, int level, sf::Vector2f pos) :
	Surge(stats, curLane, pos, FIRE_WALL), level(level)
{
	permanentHitUnits.reserve(15);
	width = FW_WIDTH;
	timeLeft = FW_TIMER;
	sprite.setOrigin({ 16, 32 });
	animationState = SurgeAnimationStates::START_UP;
	fireWallAni[0].reset(aniTime, currentFrame, sprite);
}
OrbitalStrike::OrbitalStrike(const UnitStats* stats, int curLane, sf::Vector2f pos) :
	Surge(stats, curLane, pos, ORBITAL_STRIKE)
{
	width = OS_WIDTH;
	sprite.setOrigin({ 48,48 });
	animationState = SurgeAnimationStates::ACTIVE;
	orbitalStrikeAni.reset(aniTime, currentFrame, sprite);
}

void ShockWave::start_animation(SurgeAnimationStates newState) {
	animationState = newState;
	shockWaveAni[static_cast<int>(newState) - 1].reset(aniTime, currentFrame, sprite);
};
void FireWall::start_animation(SurgeAnimationStates newState) {
	animationState = newState;
	fireWallAni[static_cast<int>(newState)].reset(aniTime, currentFrame, sprite);
};
int ShockWave::draw(sf::RenderWindow& window, float deltaTime) {
	auto events = shockWaveAni[static_cast<int>(animationState) - 1].update(aniTime, currentFrame, deltaTime, sprite);
	sprite.setPosition(pos);
	window.draw(sprite);

	return events;
}
int FireWall::draw(sf::RenderWindow& window, float deltaTime) {
	auto events = fireWallAni[static_cast<int>(animationState)].update(aniTime, currentFrame, deltaTime, sprite);
	window.draw(sprite);
	return events;
}
int OrbitalStrike::draw(sf::RenderWindow& window, float deltaTime) {
	auto events = orbitalStrikeAni.update(aniTime, currentFrame, deltaTime, sprite);
	window.draw(sprite);
	return events;
}

bool in_surge_range(float enemyXPos, float xPos, float range){ 
	return enemyXPos >= xPos - range && enemyXPos <= xPos + range;
}
bool Surge::try_terminate_unit(Unit& enemyUnit) {
	if (!stats->has_augment(TERMINATE)) return false;
	float threshold = stats->get_augment(TERMINATE).value;
	float curHpPercent = enemyUnit.hp / enemyUnit.stats->maxHp;

	return curHpPercent <= threshold;
}
void Surge::on_kill(Unit& unit) {
	if (unit.trigger_augment(stats, AugmentType::PLUNDER)) unit.statuses |= PLUNDER;
	if (stats->has_augment(CODE_BREAKER)) unit.statuses |= CODE_BREAKER;
	unit.causeOfDeath = DeathCause::SURGE;
};
void Surge::attack_units(Lane& lanes) {
	std::vector<Unit>& enemyUnits = lanes.get_targets(stats->team);

	for (auto it = enemyUnits.begin(); it != enemyUnits.end(); ++it) {
		if (already_hit_unit(it->id) || it->invincible() || it->pending_death() ||
			!(in_surge_range(it->pos.x, pos.x, width / 2))) continue;

		if (immune_to_surge_type(it->stats->immunities)) {
			if (it->stats->surge_blocker()) {
				Tween::cancel(&pos);
				readyForRemoval = true;
				return;
			}
			else continue;
		}

		if (it->take_damage(*this)) on_kill(*it);
			hitUnits.push_back(it->id);
	}
}
void ShockWave::tick(sf::RenderWindow& window, float deltaTime, Stage& stage) {
	auto events = draw(window, deltaTime);
	attack_units(stage.get_lane(currentLane));

	switch (animationState) {
	case SurgeAnimationStates::ACTIVE:
		if (!stage.tweening(id))
			start_animation(SurgeAnimationStates::ENDING);
		else {
			pos = stage.unitTweens[id].update_and_get(deltaTime);
			if (stage.unitTweens[id].isComplete) stage.cancel_tween(id);
		}
		break;
	case SurgeAnimationStates::ENDING:
		if (Animation::check_for_event(AnimationEvent::FINAL_FRAME, events))
			readyForRemoval = true;
	}
}
void FireWall::tick(sf::RenderWindow& window, float deltaTime, Stage& stage) {
	auto events = draw(window, deltaTime);

	switch (animationState) {
	case SurgeAnimationStates::START_UP:
		if (Animation::check_for_event(AnimationEvent::FINAL_FRAME, events))
			start_animation(SurgeAnimationStates::ACTIVE);
		break;
	case SurgeAnimationStates::ACTIVE:
		timeLeft -= deltaTime;
		attack_units(stage.get_lane(currentLane));

		if (timeLeft < 0) {
			timeLeft = FW_TIMER;
			if (--level > 0) {
				permanentHitUnits.insert(permanentHitUnits.end(), hitUnits.begin(), hitUnits.end());
				hitUnits.clear();
			}
			else start_animation(SurgeAnimationStates::ENDING);
		}
		break;
	case SurgeAnimationStates::ENDING:
		if (Animation::check_for_event(AnimationEvent::FINAL_FRAME, events)) 
			readyForRemoval = true;
	}
}
void OrbitalStrike::tick(sf::RenderWindow& window, float deltaTime, Stage& stage)  {
	auto events = draw(window, deltaTime);

	if (Animation::check_for_event(AnimationEvent::ATTACK, events))
		for (int i = 0; i < stage.laneCount; i++)
			attack_units(stage.get_lane(i));
	else if (Animation::check_for_event(AnimationEvent::FINAL_FRAME, events))
		readyForRemoval = true;
}

bool FireWall::never_hit_unit(int id) {
	return !already_hit_unit(id) && 
		std::find(permanentHitUnits.begin(), permanentHitUnits.end(), id) == permanentHitUnits.end();
}
void Surge::init_animations() {
	std::vector<std::pair<int, AnimationEvent>> events;
	//Animation::Animation(std::string spritePath, int frameCount, float fps, int textureSizes[2], 
	//	int cellSizes[2], std::vector<std::pair<int, AnimationEvent>> events, bool loops)

	// SHOCK WAVE
	std::string spritePath = "sprites/surges/wave_active.png";
	int textureSize[2] = { 320, 32 }, cellSize[2] = { 32,32 };
	shockWaveAni[0] = Animation(spritePath, 10, .1f, textureSize, cellSize, events, true);
	spritePath = "sprites/surges/wave_ending.png";
	textureSize[0] = 256;
	shockWaveAni[1] = Animation(spritePath, 8, .1f, textureSize, cellSize, events, false);

	// FIRE WALL
	spritePath = "sprites/surges/surge_start_up.png";
	int fw_textureSize[2] = { 384, 32 };
	fireWallAni[0] = Animation(spritePath, 12, .05f, fw_textureSize, cellSize, events, false);
	spritePath = "sprites/surges/surge_active.png";
	fireWallAni[1] = Animation(spritePath, 8, .1f, textureSize, cellSize, events, true);
	spritePath = "sprites/surges/surge_ending.png";
	fw_textureSize[0] = 288;
	fireWallAni[2] = Animation(spritePath, 9, .05f, fw_textureSize, cellSize, events, false);

	// ORBITAL STRIKE
	spritePath = "sprites/surges/orbital_strike_active.png";
	int os_textureSize[2] = { 1152, 96 }, os_cellSize[2] = { 96,96 };
	events.emplace_back(4, AnimationEvent::ATTACK);
	orbitalStrikeAni = Animation(spritePath, 12, .2f, os_textureSize, os_cellSize, events, false);
	// errpr?
}
