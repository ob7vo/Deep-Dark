#include "Projectile.h"
#include "Lane.h"

ProjectileConfig::ProjectileConfig(int id, float mag) {
	const nlohmann::json projJson = ProjData::get_proj_json(id);
	stats.setup(projJson);
	stats.dmg = (int)(stats.dmg * mag);

	int state = 0;
	for (auto& animData : projJson["animations"]) {
		std::string name = state == 0 ? "active" : "destroyed";
		std::string path = ProjData::get_proj_path(id);
		std::string fullPath = path + name + ".png";
		bool loops = !state;

		// using (create_unit_animation) still works lmao)
		aniArr[state] = Animation::create_unit_animation(animData, name, fullPath, loops);
		if (++state >= 2) break;
	}
}
int Projectile::update_hits_and_animation(float deltaTime) {
	hitCountTime += deltaTime;
	maxLifespan -= deltaTime;

	while (hitCountTime >= 1.f) {
		hitCountTime -= 1.f;
		hitsLeft--;
		//std::cout << "hit count Time ticked. HITSLEFT = " << hitsLeft << std::endl;
	}

	if (aniState == ACTIVE_STATE && (hitsLeft <= 0 || maxLifespan <= 0))
		enter_destroyed_state();
	
	return (*aniArr)[aniState].update(aniTime, frame, deltaTime, sprite);
}

void Projectile::tick(Lane& lane, float deltaTime) {
	int events = update_hits_and_animation(deltaTime);
	
	if (aniState == DESTROYED_STATE) {
		readyForRemoval = events & AnimationEvent::FINAL_FRAME;
		//std::cout << "is destroyed, ready to remove? " << readyForRemoval << std::endl;
		return;
	}

	pos = pathing->move(deltaTime);
	sprite.setPosition(pos);
	//std::cout << "projectile pos: (" << pos.x << ", " << pos.y << ")\n";

	update_hit_times(deltaTime);
	attack_units(lane);
	if (pathing->get_type() == PathingType::PROJECTILE)
		detect_lane_collision(lane);
}
void Projectile::attack_units(Lane& lane) {
	for (auto& unit : lane.get_targets(stats->team)) {
		if (!valid_target(unit)) continue;
		std::pair<int, float> hitTime = { unit.id,2.5f };
		hitUnits.push_back(hitTime);

		bool shove = stats->aug.augType & SHOVE;
		unit.take_damage(stats->dmg, shove);

		if (unit.can_proc_status(stats->aug))
			unit.add_status_effect(stats->aug);

		hitsLeft -= 1 + unit.has_augment(CHIP);
		if (hitsLeft <= 0) {
			enter_destroyed_state();
			return;
		}
	}
}
void Projectile::detect_lane_collision(Lane& lane) {
//	if (!pathing->moving_down()) return;
}
bool Projectile::within_bounds(sf::Vector2f p, float h, float w) {
    bool overlapX = pos.x < p.x + w && pos.x + stats->width > p.x;
    bool overlapY = pos.y < p.y + h && pos.y + stats->height > p.y;

    return overlapX && overlapY;
}
bool Projectile::valid_target(Unit& enemy) {
	return !enemy.invincible() && !enemy.pending_death() &&
		within_bounds(enemy.pos) && can_hit_again(enemy.id);
}
