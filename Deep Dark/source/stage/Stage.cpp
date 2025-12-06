#include "pch.h"
#include "Stage.h"
#include "Unit.h"
#include "StageRecord.h"
#include "Utils.h"

const int MAX_SUMMONS = 10;
const int MAX_SURGES = 13;
const int MAX_PROJECTILES = 10;

using json = nlohmann::json;

Stage::Stage(const json& stageSetJson, StageRecord* rec) : recorder(rec),
	enemyBase(stageSetJson["enemy_base"], -1), playerBase(stageSetJson["player_base"], 1)
{
	laneCount = stageSetJson["lane_count"];
	lanes.reserve(laneCount);
	surges.reserve(MAX_SURGES);
	projectiles.reserve(MAX_PROJECTILES);

	for (int i = 0; i < laneCount; i++) 
		lanes.emplace_back(stageSetJson["lanes"][i], i);

	std::sort(lanes.begin(), lanes.end(), [](const Lane& a, const Lane& b) {
		return a.yPos > b.yPos;
	});

	if (stageSetJson.contains("teleporters")) 
		for (auto& tp : stageSetJson["teleporters"]) {
			int lane = tp["lane"];
			sf::Vector2f tpPos = { tp["x_position"], lanes[lane].yPos };

			teleporters.emplace_back(tp, tpPos, lane);
		}
	

	if (stageSetJson.contains("traps")) 
		for (auto& trap : stageSetJson["traps"]) {
			int lane = trap["lane"];
			sf::Vector2f trapPos = { trap["x_position"], lanes[lane].yPos };

			traps.emplace_back(trap, trapPos, lane);
		}
	
	for (auto& spawnData : stageSetJson["enemy_spawns"]) 
		enemySpawners.emplace_back(spawnData, *this);
	

	break_spawner_thresholds();
}
MoveRequest::MoveRequest(const Unit& unit, int newLane, float axisPos, RequestType type) :
unitId(unit.id), currentLane(unit.get_lane()), newLane(newLane), team(unit.stats->team),
axisPos(axisPos), type(type) {}

void Stage::break_spawner_thresholds() {
	float percentage = enemyBase.get_hp_percentage();

	for (auto& spawner : enemySpawners) {
		if (spawner.currentSpawnIndex >= 0 || percentage > spawner.percentThreshold) continue;

		spawner.currentSpawnIndex = 0;
		spawner.nextSpawnTime -= (INACTIVE_SPAWNER - timeSinceStart);
	}
}
Lane& Stage::get_closest_lane(float y) {
	int closest = 0;
	float minDist = abs(lanes[0].yPos - y);

	for (int i = 1; i < laneCount; i++) {
		float dist = abs(lanes[i].yPos - y);
		if (dist < minDist) {
			minDist = dist;
			closest = i;
		}
	}

	return lanes[closest];
}

// creation
Unit* Stage::create_unit(int laneIndex, const UnitStats* unitStats, UnitAniMap* aniMap) {
	if (laneIndex < 0 || laneIndex >= lanes.size()) {
		std::cout << "Lane Index " << laneIndex << " is out of range, cannot spawn Unit" << std::endl;
		return nullptr;
	}

	nextUnitID++;
	recorder->add_spawn(unitStats->team, laneIndex);
	sf::Vector2f spawnPos = lanes[laneIndex].get_spawn_pos(unitStats->team);

	auto& newVec = get_source_vector(laneIndex, unitStats->team);

	return &newVec.emplace_back(this, spawnPos, laneIndex,
		unitStats, aniMap, nextUnitID);
}
void Stage::try_revive_unit(UnitSpawner* spawner) {
	// if they dont have the clone ability, or have be inflicted with code breaker, return
	Unit* newUnit = create_unit(spawner->laneInd, spawner->stats, spawner->aniMap);
	if (newUnit) {
		int maxHp = spawner->stats->maxHp;
		auto newHp = (int)((float)maxHp * spawner->stats->get_augment(CLONE).value);
		newUnit->status.hp = newHp;

		for (int i = 1; i <= spawner->stats->knockbacks; i++) {
			int threshold = maxHp - (maxHp * i / spawner->stats->knockbacks);
			if (newHp <= threshold)
				newUnit->status.kbIndex = i + 1;
		}
		std::cout << "cloned unit, kbIndex (" << newUnit->status.kbIndex  <<
			"/" << spawner->stats->knockbacks << ")" << std::endl;

		newUnit->status.statusFlags |= CODE_BREAKER;
		newUnit->movement.pos = spawner->sprite.getPosition();
		newUnit->anim.start(UnitAnimationState::MOVE);
	}
}
void Stage::create_summon(const Unit& unit) {
	Augment salvage = unit.stats->get_augment(SALVAGE);
	int id = salvage.intValue;

	if (!can_summon(id, salvage.value2)) return;

	float spawnRange = salvage.value;
	float xPos = unit.get_pos().x;
	float newX = Random::r_float(xPos - spawnRange, xPos + spawnRange);
	sf::Vector2f newPos = { newX, unit.get_pos().y};

	Unit* summon = create_unit(unit.get_lane(), &summonData->stats, &summonData->ani);
	if (summon) {
		summonData->count++;
		summon->movement.pos = newPos;
		summon->spawnCategory = SpawnCategory::SUMMON;
	}
}

Surge* Stage::create_surge(const Unit& unit, const Augment& surge) {
	int lane = unit.get_lane();
	int level = surge.surgeLevel;
	sf::Vector2f pos = unit.get_pos();

	if (surge.augType != AugmentType::SHOCK_WAVE) {
		float min = surge.value;
		float max = std::max(min, surge.value2);
		pos.x += Random::r_float(min, max) * static_cast<float>(unit.stats->team);
	}

	Surge* pSurge = create_surge(unit.stats, lane, level, pos, surge.augType);
	pSurge->hitIndex = unit.combat.hitIndex;
	return surges.back().get();
}
void Stage::create_surge(const BaseCannon* cannon, const Augment& surge) {
	Surge* pSurge = create_surge(&cannon->cannonStats, selectedLane, surge.surgeLevel, cannon->pos, surge.augType);
	if (pSurge) pSurge->set_as_cannon_creation();
}
void Stage::create_surge(const BaseCannon* cannon, const Augment& surge, int lane, float distance) {
	sf::Vector2f pos = cannon->pos;
	pos.x -= distance;

	Surge* pSurge = create_surge(&cannon->cannonStats, lane, surge.surgeLevel, pos, surge.augType);
	if (pSurge) pSurge->set_as_cannon_creation();
}
Surge* Stage::create_surge(const UnitStats* stats, int lane, int level, sf::Vector2f pos, AugmentType aug) {
	switch (aug) {
	case AugmentType::SHOCK_WAVE:
		return surges.emplace_back(std::make_unique<ShockWave>(stats, lane, level, pos, *this)).get();
	case AugmentType::FIRE_WALL:
		return surges.emplace_back(std::make_unique<FireWall>(stats, lane, level, pos)).get();
	case AugmentType::ORBITAL_STRIKE:
		return surges.emplace_back(std::make_unique<OrbitalStrike>(stats, lane, pos)).get();
	default:
		std::cout << "no surge was created. invalid augment type" << std::endl;
		return nullptr;
	}
	return nullptr;
}

void Stage::create_projectile(const Unit& unit, const Augment& aug) {
	if (projectiles.size() >= MAX_PROJECTILES) {
		std::cout << "too many projectiles to add more" << std::endl;
		return;
	}

	int id = aug.intValue;
	if (!projConfigs.contains(id)) projConfigs[id] = ProjectileConfig(id);

	ProjectileConfig& config = projConfigs[id];
	const nlohmann::json projJson = ProjData::get_proj_json(id);
	Projectile& proj = projectiles.emplace_back(projConfigs[id]);

	char pathingTypeChar = projJson["pathing"]["type"].get<std::string>()[0];
	switch (pathingTypeChar) {
	case 'c': {
		sf::Vector2f center = { unit.get_pos().x + (aug.value * (float)unit.stats->team), unit.get_pos().y};
		float speed = projJson["pathing"]["speed"];
		float radius = projJson["pathing"]["radius"];
		float startingAngle = projJson["pathing"].value("angle", 0.f);
		proj.pathing = std::make_unique<CirclePathing>(center, speed, radius, startingAngle);
		break;
	}
	case 'p': {
		sf::Vector2f velocity = { projJson["pathing"]["velocity"][0], projJson["pathing"]["velocity"][1] };
		float mass = projJson["pathing"]["mass"];
		proj.pathing = std::make_unique<ProjectilePathing>(unit.get_pos(), velocity, mass);
		break;
	}
	case 'w': {
		std::vector<sf::Vector2f> waypoints = {};
		for (const auto& wp : projJson["pathing"]["waypoints"])
			waypoints.push_back(unit.get_pos() + sf::Vector2f(wp["x"], wp["y"]));

		float timer = projJson["pathing"]["time"];
		int loops = projJson["pathing"]["loops"];
		auto easeType = (EasingType)projJson["pathing"].value("ease_type", 0);

		proj.pathing = std::make_unique<WaypointPathing>(waypoints, loops, timer, easeType);
		break;
	}
	default:
		std::cout << "could not create projective. Char: " << pathingTypeChar << std::endl;
		return;
	}

	std::cout << "created projectile" << std::endl;
}
void Stage::create_hitbox_visualizers(sf::Vector2f pos, std::pair<float, float> range, int team) {
	float width = range.second - range.first;
	sf::RectangleShape shape({ width, HITBOX_HEIGHT });

	float originX = team == PLAYER_TEAM ? 0 : width;
	float posX = pos.x + (range.first * static_cast<float>(team));
	shape.setOrigin({ originX, HITBOX_HEIGHT });
	shape.setFillColor(HITBOX_COLOR);
	shape.setPosition({ posX, pos.y });

	hitboxes.emplace_back(shape, HITBOX_TIMER );
}

std::pair<float, int> Stage::find_lane_to_fall_on(const Unit& unit) {
	float newY = FLOOR;

	const auto& [leftHurtboxEdge, rightHurtboxEdge] = unit.getHurtboxEdges();

	for (int i = unit.get_lane() - 1; i >= 0; i--) {
		if (!lanes[i].within_gap(leftHurtboxEdge, rightHurtboxEdge)) {
			newY = lanes[i].yPos;
			return { newY, i };
		}
	}

	return { newY, -1 };
}
int Stage::find_lane_to_knock_to(const Unit& unit, int inc) const {
	const auto& [leftHurtboxEdge, rightHurtboxEdge] = unit.getHurtboxEdges();

	for (int i = unit.get_lane() + inc; i >= 0 && i < laneCount; i += inc) {
		if (lanes[i].within_gap(leftHurtboxEdge, rightHurtboxEdge)) continue;

		return i;
	}

	return unit.get_lane();
}
bool Stage::can_summon(int summonId, float magnification) {
	if (summonData)
		return summonData->count < MAX_SUMMONS ? true : false;

	nlohmann::json unitJson = UnitData::createUnitJson(summonId);
	summonData = std::make_unique<SummonData>(unitJson, magnification);

	return summonData ? true : false;
}

void MoveRequest::move_unit_by_request(Unit& unit, Stage& stage) const{
	unit.movement.currentLane = newLane;

	switch (type) { 
	case RequestType::FALL:
		unit.movement.fall(unit, axisPos);
		break;
	case RequestType::SQUASH:
		unit.movement.squash(unit, axisPos);
		break;
	case RequestType::JUMP:
		unit.movement.jump(unit, axisPos);
		break;
	case RequestType::TELEPORT:
		unit.movement.pos = { axisPos, stage.lanes[newLane].yPos };
		break;
	default:
		unit.movement.launch(unit, axisPos);
		break;
	}
}

