#pragma once
#include "SFML/System/Vector2.hpp"

namespace UI {
	namespace StartMenu {
		// Buttons
		sf::Vector2f QUIT_BTN_POS = { 0.35f, 0.65f };
		sf::Vector2f QUIT_BTN_SIZE = { 0.15f, 0.2f };

		sf::Vector2f START_BTN_POS = { 0.65f, 0.65f };
		sf::Vector2f START_BTN_SIZE = { 0.15f, 0.2f };

		sf::Vector2f SETTINGS_BTN_POS = { 0.5f, 0.5f };
		sf::Vector2f SETTINGS_BTN_SIZE = { 0.15f, 0.2f };

		// Texts
		sf::Vector2f MENU_TEXT_POS = { 0.5f, 0.35f }; 
	}
	namespace Stage {
		// Buttons
		const sf::Vector2f PAUSE_BTN_POS = { 0.15f,0.25f };
		const sf::Vector2f PAUSE_BTN_SIZE = { 0.1f,0.1f };

		const sf::Vector2f CANNON_BTN_POS = { 0.8f,0.8f };
		const sf::Vector2f CANNON_BTN_SIZE = { 0.1f,0.1f };

		const sf::Vector2f BAG_BTN_POS = { 0.12f,0.85f };
		const sf::Vector2f BAG_BTN_SIZE = { 0.1f,0.1f };

		const sf::Vector2f CLOSE_GAME_BTN_POS = { 0.4f, 0.7f };
		const sf::Vector2f CLOSE_GAME_BTN_SIZE = { 0.1f, 0.1f };

		const sf::Vector2f CLOSE_MENU_BTN_POS = { .6f, .7f };
		const sf::Vector2f CLOSE_MENU_BTN_SIZE = { 0.1f, 0.1f };

		// Texts
		const sf::Vector2f PARTS_TEXT_POS = { 0.1f,0.1f };
		const sf::Vector2f CHALLENGES_TEXT_POS = { 0.7f,0.8f };
		const sf::Vector2f BAG_COST_TEXT_POS = { 0.05f,0.9f };
		const sf::Vector2f PAUSE_TEXT_POS = { 0.5f, 0.35f };

		// Sprites
		const sf::Vector2f PAUSE_MENU_POS = { 0.5f,0.5f };
		const sf::Vector2f PAUSE_MENU_SIZE = { 0.2f,0.2f };

		const sf::Vector2f FIRST_UNIT_SLOT_POS = { 0.278f, 0.84375f };
		const sf::Vector2f UNIT_SLOT_INCREMENT = { 0.089f, 0.0625f };
	}
}
