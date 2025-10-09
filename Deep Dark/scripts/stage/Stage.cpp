#include "Stage.h"
#include "Unit.h"
#include <iostream>
#include <fstream>

const int MAX_SUMMONS = 10;

using json = nlohmann::json;
/*float firstSpawnTime, float minDelay,
	float maxDelay, int totalSpawns, float magnification, std::vector<int> laneIndexes,
	bool inf, std::vector<std::pair<float,int>> forcedSpawns = {})
	: totalSpawns(totalSpawns), minSpawnDelay(minDelay), maxSpawnDelay(maxDelay), forcedSpawnTimes(forcedSpawns),
	laneToSpawn(laneIndexes), firstSpawnTime(firstSpawnTime), enemyStats(jsonFile, magnification), infinite(inf) */
EnemySpawner::EnemySpawner(const json& jsonFile, const json& file){
	totalSpawns = file["total_spawns"];
	float magnification = file["magnification"];
	firstSpawnTime = file["first_spawn_time"];
	spawnDelays = file["spawn_delays"];
	std::vector<int> laneIndexes = file["lane_indexes"];
	laneSpawnIndexes = laneIndexes;
	infinite = file.contains("infinite");

	if (file.contains("forced_spawn_times"))
		for (auto& forcedTime : file["forced_spawn_times"])
			forcedSpawnTimes.emplace_back(forcedTime[0], forcedTime[1]);
	nextSpawnTime = firstSpawnTime;

	Animation::create_unit_animation_array(jsonFile, aniArr);
	enemyStats = UnitStats(jsonFile, magnification);
}
Stage::Stage(const json& stageFile, StageRecord& rec) : recorder(rec),
	enemyBase(stageFile, -1), playerBase(stageFile, 1)
{
	laneCount = stageFile["lane_count"];
	lanes.reserve(laneCount);
	surges.reserve(10);

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
		std::string path = spawnData["json_path"];
		std::ifstream file(path);
		json enemyJson = json::parse(file);
		file.close();

		enemySpawners.emplace_back(enemyJson, spawnData);
	}
}
MoveRequest::MoveRequest(Unit& unit, int newLane, float fallTo, RequestType type) :
unitId(unit.id), team(unit.stats->team), currentLane(unit.currentLane),
newLane(newLane), pos(fallTo), type(type) {}
float ran_num(float min, float max) {
	static std::minstd_rand gen(std::random_device{}());
	std::uniform_real_distribution<float> dis(min, max);
	return dis(gen);
}

// creation
Unit* Stage::create_unit(int laneIndex, const UnitStats* unitStats, std::array<Animation, 5>* aniMap) {
	if (laneIndex < 0 || laneIndex >= lanes.size()) {
		std::cout << "Lane Index " << laneIndex << " is out of range, cannot spawn Unit" << std::endl;
		return nullptr;
	}

	int team = unitStats->team;;
	sf::Vector2f spawnPos = lanes[laneIndex].get_spawn_pos(team);

	//	std::cout << "emplacing back new unit" << std::endl;
	auto& newVec = get_source_vector(laneIndex, team);

	return &newVec.emplace_back(this, spawnPos, laneIndex, unitStats, aniMap, nextUnitID++);
}
void Stage::try_revive_unit(UnitSpawner* spawner) {
	// if they dont have the clone ability, or have be inflicted with code breaker, return
	Unit* newUnit = create_unit(spawner->lane, spawner->stats, spawner->aniMap);
	if (newUnit) {
		int maxHp = spawner->stats->maxHp;
		int newHp = maxHp * spawner->stats->get_augment(CLONE).value;
		newUnit->hp = newHp;
		for (int i = 1; i <= spawner->stats->knockbacks; i++) {
			int threshold = maxHp - (maxHp * i / spawner->stats->knockbacks);
			if (newHp <= threshold)
				newUnit->kbIndex = i + 1;
		}
		std::cout << "cloned unit, kbIndex = " << newUnit->kbIndex << std::endl;

		newUnit->statuses |= CODE_BREAKER;
		newUnit->pos = spawner->pos;
		newUnit->start_animation(UnitAnimationState::MOVING);
	}
}
Summon* Stage::try_create_summon_data(int summonId, float magnification) {
	if (summonData) 
		return summonData->count < MAX_SUMMONS ? summonData.get() : nullptr;

	switch (summonId) {
	case 103:
		std::ifstream file("configs/enemy_units/slime/slime.json");
		nlohmann::json unitJson = nlohmann::json::parse(file);
		summonData = std::make_unique<Summon>(unitJson, magnification);
		break;
	}
	return summonData ? summonData.get() : nullptr;
}
void Stage::create_summon(Unit& unit) {
	Augment salvage = unit.stats->get_augment(SALVAGE);
	int id = salvage.intValue;
	Summon* summonData = try_create_summon_data(id, unit.stats->magnification);
	if (!summonData) return;

	float spawnRange = salvage.value;
	float newX = ran_num(unit.pos.x - spawnRange, unit.pos.x + spawnRange);
	sf::Vector2f newPos = { newX, unit.pos.y };
	Unit* summon = create_unit(unit.currentLane, &summonData->data.stats, &summonData->data.ani);
	if (summon) {
		summonData++;
		summon->pos = newPos;
		summon->spawnCategory = SpawnCategory::SUMMON;
	}
}

// units
void Stage::create_surge(Unit& unit, const Augment& surge) {
	int lane = unit.currentLane;
	int level = surge.surgeLevel;
	sf::Vector2f pos = unit.pos;

	if (surge.augType != AugmentType::SHOCK_WAVE)
		pos.x += surge.value * unit.stats->team;

	create_surge(unit.stats, lane, level, pos, surge.augType);
	surges.back()->hitIndex = unit.hitIndex;
}
// player bases
void Stage::create_surge(BaseCannon* pCannon, const Augment& surge) {
	create_surge(&pCannon->cannonStats, selectedLane, surge.surgeLevel, pCannon->pos, surge.augType);
}
// enemy bases
void Stage::create_surge(BaseCannon* eCannon, const Augment& surge, int lane, float distance) {
	sf::Vector2f pos = eCannon->pos;
	pos.x -= distance;
	create_surge(&eCannon->cannonStats, lane, surge.surgeLevel, pos, surge.augType);
}
void Stage::create_surge(const UnitStats* stats, int lane, int level, sf::Vector2f pos, AugmentType aug) {
	switch (aug) {
	case AugmentType::SHOCK_WAVE:
		surges.emplace_back(std::make_unique<ShockWave>(stats, lane, level, pos, *this));
		break;
	case AugmentType::FIRE_WALL:
		surges.emplace_back(std::make_unique<FireWall>(stats, lane, level, pos));
		break;
	case AugmentType::ORBITAL_STRIKE:
		surges.emplace_back(std::make_unique<OrbitalStrike>(stats, lane, pos));
		break;
	}
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

void Stage::only_draw(sf::RenderWindow& window) {
	for (auto& lane : lanes) {
		lane.draw(window);
		for (auto it = lane.enemyUnits.begin(); it != lane.enemyUnits.end();) {
			window.draw(it->get_sprite());
			++it;
		}
		for (auto it = lane.playerUnits.begin(); it != lane.playerUnits.end();) {
			window.draw(it->get_sprite());
			++it;
		}
	}
}

//	else if (lane.playerTeleporter && lane.playerTeleporter->check_if_on_teleporter(it->pos.x))
	//	teleport_unit(lane.playerTeleporter, it);
