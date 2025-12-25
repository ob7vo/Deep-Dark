#include "pch.h"
#include "EntityTextures.h"
#include "Utils.h"

using namespace FolderPaths;

namespace Textures::Entity {
	// Spawners
	sf::Texture t_cloner = createTexture(path(entitySpritesPath, "cloner.png"));
	sf::Texture t_dropbox = createTexture(path(entitySpritesPath, "drop_box.png"));
	sf::Texture t_surgeSpawner = createTexture(path(entitySpritesPath, "surge_spawner.png"));

	// Traps
	sf::Texture t_teleporter = createTexture(path(trapSpritesPath, "teleporter.png"));
	sf::Texture t_flatDmgTrap = createTexture(path(trapSpritesPath, "flat_dmg_trap.png"));
	sf::Texture t_launchPadTrap = createTexture(path(trapSpritesPath, "launch_pad.png"));
	sf::Texture t_trapDoor = createTexture(path(trapSpritesPath, "trap_door.png"));

	// Surges
	sf::Texture t_shockWaveActive = createTexture(path(surgeSpritesPath, "shockwave_active.png"));
	sf::Texture t_shockWaveEnding = createTexture(path(surgeSpritesPath, "shockwave_ending.png"));
	sf::Texture t_fireWallStartUp = createTexture(path(surgeSpritesPath, "firewall_startup.png"));
	sf::Texture t_fireWallActive = createTexture(path(surgeSpritesPath, "firewall_active.png"));
	sf::Texture t_fireWallEnding = createTexture(path(surgeSpritesPath, "firewall_ending.png"));
	sf::Texture t_orbitalStrikeActive = createTexture(path(surgeSpritesPath, "orbital_strike_active.png"));
}