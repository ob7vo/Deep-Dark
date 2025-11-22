#include "Stage.h"
#include "Unit.h"
#include <iostream>
#include <fstream>

const int MAX_SUMMONS = 10;
const int MAX_SURGES = 13;
const int MAX_PROJECTILES = 10;

using json = nlohmann::json;

EnemySpawner::EnemySpawner(const json& spawnerData, Stage& stage){
	currentSpawnIndex = -1;
	totalSpawns = spawnerData["total_spawns"];
	float magnification = spawnerData["magnification"];
	float firstSpawnTime = spawnerData["first_spawn_time"];
	spawnDelays = spawnerData["spawn_delays"].get<std::pair<float,float>>(); // min-max pair.
	std::vector<int> laneIndexes = spawnerData["lane_indexes"];
	laneSpawnIndexes = laneIndexes;
	infinite = spawnerData.contains("infinite");
	percentThreshold = spawnerData.value("percent_threshold", 101.0f); 

	if (spawnerData.contains("forced_spawn_times"))
		for (auto& forcedTime : spawnerData["forced_spawn_times"])
			forcedSpawnTimes.emplace_back((float)forcedTime[0], (int)forcedTime[1]);
	nextSpawnTime = firstSpawnTime + INACTIVE_SPAWNER;

	int id = spawnerData["unit_id"];
	const json unitFile = UnitData::get_unit_json(id);
	Animation::setup_unit_animation_map(unitFile, aniArr);
	enemyStats = UnitStats::enemy(unitFile, magnification);

	for (auto& aug : enemyStats.augments) 
		if (aug.augType & PROJECTILE) 
			stage.projConfigs[id] = ProjectileConfig(id, magnification);
}
void Stage::break_spawner_thresholds(float timeSinceStart) {
	float percentage = enemyBase.get_hp_percentage();
	enemyBase.tookDmgThisFrame = false;

	std::cout << "updating bade percentage\n";
	for (auto& spawner : enemySpawners) {
		if (spawner.currentSpawnIndex >= 0) {
			std::cout << "Spawn Index is >= 0: " << spawner.currentSpawnIndex << std::endl;
			continue;
		}
		if (percentage > spawner.percentThreshold) {
			std::cout << "threshold not met. SpawnerThreshold: [" << spawner.percentThreshold
				<< "] - Current Base Percent: " << percentage << std::endl;
			continue;
		}
	//	if (spawner.currentSpawnIndex >= 0 ||
	//		percentage > spawner.percentThreshold) continue;
		spawner.currentSpawnIndex = 0;
		spawner.nextSpawnTime -= (INACTIVE_SPAWNER - timeSinceStart);
		std::cout << "Activating spawner. First spawn at: " <<
			spawner.nextSpawnTime << std::endl;
	}
}
	
Stage::Stage(const json& stageFile, StageRecord* rec) : recorder(rec),
	enemyBase(stageFile, -1), playerBase(stageFile, 1)
{
	laneCount = stageFile["lane_count"];
	lanes.reserve(laneCount);
	surges.reserve(MAX_SURGES);
	projectiles.reserve(MAX_PROJECTILES);

	for (int i = 0; i < laneCount; i++) {
		auto& lane = stageFile["lanes"][i];
		float playerXSpawn = lane["player_spawn_position"];
		float enemyXSpawn = lane["enemy_spawn_position"];
		float yPos = lane["y_position"];

		std::vector<std::pair<float, float>> gaps;
		if (lane.contains("gaps")) 
			for (auto& gap : lane["gaps"]) {
				if (gap[1] <= gap[0])
					std::cout << "Invalid gap" << std::endl;
				else
					gaps.push_back({ gap[0], gap[1] });
			}

		lanes.emplace_back(enemyXSpawn, playerXSpawn, yPos, gaps, i);
	}

	std::sort(lanes.begin(), lanes.end(), [](const Lane& a, const Lane& b) {
		return a.yPos > b.yPos;
	});

	if (stageFile.contains("teleporters")) {
		for (auto& tp : stageFile["teleporters"]) {
			int lane = tp["lane"];
			int connectedLane = tp["connected_lane"];
			bool isPlayerTeleporter = tp["player_team"];
			float xPos = tp["x_position"];
			float xDestination = tp["x_destination"];
			float y = lanes[lane].yPos;

			if (isPlayerTeleporter) 
				lanes[lane].playerTeleporter = std::make_unique<Teleporter>(connectedLane, xPos, xDestination, y);
			else 
				lanes[lane].enemyTeleporter = std::make_unique<Teleporter>(connectedLane, xPos, xDestination, y);
		}
	}
	if (stageFile.contains("traps")) {
		for (auto& trap : stageFile["traps"]) {
			int lane = trap["lane"];
			lanes[lane].trap = std::make_unique<Trap>(lanes[lane], trap);
		}
	}
	for (auto& spawnData : stageFile["enemy_spawns"]) {
		enemySpawners.emplace_back(spawnData, *this);
	}

	break_spawner_thresholds();
}
MoveRequest::MoveRequest(Unit& unit, int newLane, float axisPos, RequestType type) :
unitId(unit.id), team(unit.stats->team), currentLane(unit.currentLane),
newLane(newLane), axisPos(axisPos), type(type) {}

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
float ran_num(float min, float max) {
	static std::minstd_rand gen(std::random_device{}());
	std::uniform_real_distribution<float> dis(min, max);
	return dis(gen);
}

// creation
Unit* Stage::create_unit(int laneIndex, const UnitStats* unitStats, UnitAniMap* aniMap) {
	if (laneIndex < 0 || laneIndex >= lanes.size()) {
		std::cout << "Lane Index " << laneIndex << " is out of range, cannot spawn Unit" << std::endl;
		return nullptr;
	}

	recorder->add_spawn(unitStats->team, laneIndex);
	sf::Vector2f spawnPos = lanes[laneIndex].get_spawn_pos(unitStats->team);

	//	std::cout << "emplacing back new unit" << std::endl;
	auto& newVec = get_source_vector(laneIndex, unitStats->team);

	return &newVec.emplace_back(this, spawnPos, laneIndex, unitStats, aniMap, nextUnitID++);
}
void Stage::try_revive_unit(UnitSpawner* spawner) {
	// if they dont have the clone ability, or have be inflicted with code breaker, return
	Unit* newUnit = create_unit(spawner->lane, spawner->stats, spawner->aniMap);
	if (newUnit) {
		int maxHp = spawner->stats->maxHp;
		int newHp = (int)(maxHp * spawner->stats->get_augment(CLONE).value);
		newUnit->hp = newHp;

		for (int i = 1; i <= spawner->stats->knockbacks; i++) {
			int threshold = maxHp - (maxHp * i / spawner->stats->knockbacks);
			if (newHp <= threshold)
				newUnit->kbIndex = i + 1;
		}
		std::cout << "cloned unit, kbIndex (" << newUnit->kbIndex  <<
			"/" << spawner->stats->knockbacks << ")" << std::endl;

		newUnit->statuses |= CODE_BREAKER;
		newUnit->pos = spawner->sprite.getPosition();
		newUnit->start_animation(UnitAnimationState::MOVE);
	}
}
SummonData* Stage::try_get_summon_data(int summonId, float magnification) {
	if (summonData) 
		return summonData->count < MAX_SUMMONS ? summonData.get() : nullptr;
	
	nlohmann::json unitJson = UnitData::get_unit_json(summonId);
	summonData = std::make_unique<SummonData>(unitJson, magnification);

	return summonData ? summonData.get() : nullptr;
}
void Stage::create_summon(Unit& unit) {
	Augment salvage = unit.stats->get_augment(SALVAGE);
	int id = salvage.intValue;

	SummonData* summonData = try_get_summon_data(id, salvage.value2);
	if (!summonData) return;

	float spawnRange = salvage.value;
	float newX = ran_num(unit.pos.x - spawnRange, unit.pos.x + spawnRange);
	sf::Vector2f newPos = { newX, unit.pos.y };

	Unit* summon = create_unit(unit.currentLane, &summonData->stats, &summonData->ani);
	if (summon) {
		summonData->count++;
		summon->pos = newPos;
		summon->spawnCategory = SpawnCategory::SUMMON;
	}
}

// units
Surge* Stage::create_surge(Unit& unit, const Augment& surge) {
	int lane = unit.currentLane;
	int level = surge.surgeLevel;
	sf::Vector2f pos = unit.pos;

	if (surge.augType != AugmentType::SHOCK_WAVE) {
		float min = surge.value;
		float max = std::max(min, surge.value2);
		pos.x += ran_num(min, max) * unit.stats->team;
	}

	Surge* pSurge = create_surge(unit.stats, lane, level, pos, surge.augType);
	pSurge->hitIndex = unit.hitIndex;
	return surges.back().get();
}
// player bases
void Stage::create_surge(BaseCannon* pCannon, const Augment& surge) {
	Surge* pSurge = create_surge(&pCannon->cannonStats, selectedLane, surge.surgeLevel, pCannon->pos, surge.augType);
	if (pSurge) pSurge->set_as_cannon_creation();
}
// enemy bases
void Stage::create_surge(BaseCannon* eCannon, const Augment& surge, int lane, float distance) {
	sf::Vector2f pos = eCannon->pos;
	pos.x -= distance;

	Surge* pSurge = create_surge(&eCannon->cannonStats, lane, surge.surgeLevel, pos, surge.augType);
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

void Stage::create_projectile(Unit& unit, const Augment& aug) {
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
		sf::Vector2f center = { unit.pos.x + (aug.value * unit.stats->team), unit.pos.y };
		float speed = projJson["pathing"]["speed"];
		float radius = projJson["pathing"]["radius"];
		float startingAngle = projJson["pathing"].value("angle", 0.f);
		proj.pathing = std::make_unique<CirclePathing>(center, speed, radius, startingAngle);
		break;
	}
	case 'p': {
		sf::Vector2f velocity = { projJson["pathing"]["velocity"][0], projJson["pathing"]["velocity"][1] };
		float mass = projJson["pathing"]["mass"];
		proj.pathing = std::make_unique<ProjectilePathing>(unit.pos, velocity, mass);
		break;
	}
	case 'w': {
		std::vector<sf::Vector2f> waypoints = {};
		for (const auto& wp : projJson["pathing"]["waypoints"])
			waypoints.push_back(unit.pos + sf::Vector2f(wp["x"], wp["y"]));
		float timer = projJson["pathing"]["time"];
		int loops = projJson["pathing"]["loops"];
		EasingType easeType = (EasingType)projJson["pathing"].value("ease_type", 0);
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
	float posX = pos.x + (range.first * team);
	shape.setOrigin({ originX, HITBOX_HEIGHT });
	shape.setFillColor(HITBOX_COLOR);
	shape.setPosition({ posX, pos.y });
	hitboxes.push_back({ shape, HITBOX_TIMER });
}

std::pair<float, int> Stage::find_lane_to_fall_on(Unit& unit) {
	float newY = FLOOR;

	for (int i = unit.currentLane - 1; i >= 0; i--) {
		Lane& lane = lanes[i];

		if (!lane.within_gap(unit.pos.x)) {
			newY = lane.yPos;
			return { newY, i };
		}
	}

	unit.hp = -100;
	unit.causeOfDeath = DeathCause::FALLING;
	return { newY, unit.currentLane };
}
int Stage::find_lane_to_knock_to(Unit& unit, int inc) {
	for (int i = unit.currentLane + inc; i >= 0 && i < laneCount; i += inc) {
		if (lanes[i].within_gap(unit.pos.x)) continue;
		return i;
	}
	return unit.currentLane;
}

void MoveRequest::move_unit_by_request(Unit& unit, Stage& stage) {
	unit.currentLane = newLane;

	switch (type) { 
	case RequestType::FALL:
		unit.fall(axisPos);
		break;
	case RequestType::SQUASH:
		unit.squash(axisPos);
		break;
	case RequestType::JUMP:
		unit.jump(axisPos);
		break;
	case RequestType::TELEPORT:
		unit.pos = { axisPos, stage.lanes[newLane].yPos };
		break;
	default:
		unit.launch(axisPos);
		break;
	}
}

//	else if (lane.playerTeleporter && lane.playerTeleporter->check_if_on_teleporter(it->pos.x))
	//	teleport_unit(lane.playerTeleporter, it);
