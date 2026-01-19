#include "pch.h"
#include "Projectile.h"
#include "Stage.h"
#include "UILayout.h"

ProjectileData::ProjectileData(int id, float mag) {
	const nlohmann::json projJson = ProjData::get_proj_json(id);
	stats.setup(projJson);
	stats.dmg *= mag;

	int state = 0;
	for (const auto& [animName, animData] : projJson["animations"].items()) {
		std::string path = ProjData::get_proj_path(id);
		std::string fullPath = path + animName + ".png";
		bool loops = !state;

		// using (create_unit_animation) still works lmao)
		Textures::loadTexture(textures[state], fullPath);
		aniArr[state] = AnimationClip::from_json(animData, &textures[state], loops);

		if (++state >= 2) break;
	}
}
Projectile::Projectile(ProjectileData& config) : 
	StageEntity({ -5000.f,-5000.f }, 0), // Calling pathing (made in Stage class) will get the start position
	stats(&config.stats), 
	aniArr(&config.aniArr) 
{
	animPlayer.start(&(*aniArr)[0], sprite);
	sprite.scale({ 0.2f,0.2f });

	hitsLeft = stats->hits;
	maxLifespan = stats->maxLifespan;

	pos = pathing->getStartingPosition();
}

void Projectile::enter_destroyed_state() {
	aniState = DESTROYED_STATE;
	animPlayer.start(&(*aniArr)[aniState], sprite);
}
AnimationEvent Projectile::update_hits_and_animation(float deltaTime) {
	hitCountTime += deltaTime;
	maxLifespan -= deltaTime;

	while (hitCountTime >= 1.f) {
		hitCountTime -= 1.f;
		hitsLeft--;
	}

	if (aniState == ACTIVE_STATE && (hitsLeft <= 0 || maxLifespan <= 0))
		enter_destroyed_state();
	
	return animPlayer.update(deltaTime, sprite);
}
void Projectile::update_hit_times(float deltaTime) {
	std::erase_if(hitUnits, [deltaTime](auto& unit) {
			unit.second -= deltaTime;
			return unit.second <= 0;
		});
}
bool Projectile::can_unit_hit_again(int id) const {
	return std::find_if(hitUnits.begin(), hitUnits.end(),
		[&id](const auto& unit) {
			return unit.first == id;
		}) == hitUnits.end();
}

void Projectile::tick(Stage& stage, float deltaTime) {
	auto events = update_hits_and_animation(deltaTime);
	
	if (aniState == DESTROYED_STATE) {
		readyForRemoval = any(events & AnimationEvent::FINAL_FRAME);
		return;
	}

	pos = pathing->move(deltaTime);
	sprite.setPosition(pos);

	update_hit_times(deltaTime);
	attack_units(stage);
}
void Projectile::draw(sf::RenderWindow& window) const {
	sf::RectangleShape hitbox({stats->height, stats->width});

	hitbox.setPosition(pos);
	hitbox.setFillColor(UI::Colors::RED_TRANSPARENT);
	hitbox.setOrigin(hitbox.getSize() * 0.5f);

	window.draw(sprite);
	window.draw(hitbox);
}

void Projectile::attack_units(Stage& stage) {
	Lane& closetLane = stage.get_closest_lane(pos.y);

	for (const auto& index : closetLane.getOpponentUnits(stats->team)) {
		auto& unit = stage.getUnit(index);

		if (!valid_target(unit)) continue;

		std::pair<int, float> hitTime = { unit.spawnID, 2.5f };
		hitUnits.push_back(hitTime);

		// Trigger shove if the projectile has the augment
		bool shove = has(stats->aug.augType & AugmentType::SHOVE);

		unit.status.take_damage(unit, stats->dmg, shove);

		// Proc any statuc effects the projectile might have
		if (unit.status.can_proc_status(unit, stats->aug))
			unit.status.add_status_effect(stats->aug);

		hitsLeft -= 1 + (int)unit.stats->has_augment(AugmentType::ROUGH);
		if (hitsLeft <= 0) {
			enter_destroyed_state();
			return;
		}
	}
}

bool Projectile::within_bounds(sf::Vector2f p) const {
    bool overlapX = pos.x < p.x + stats->width && pos.x + stats->width > p.x;
    bool overlapY = pos.y < p.y + stats->height && pos.y + stats->height > p.y;

    return overlapX && overlapY;
}
bool Projectile::valid_target(const Unit& enemy) const {
	return !enemy.anim.invincible() && within_bounds(enemy.get_pos())
		&& can_unit_hit_again(enemy.spawnID);
}
