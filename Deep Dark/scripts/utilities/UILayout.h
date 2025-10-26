#pragma once
#include "SFML/System/Vector2.hpp"

namespace UI {
	namespace StartMenu {
		// Buttons
		constexpr sf::Vector2f QUIT_BTN_POS = { 0.35f, 0.65f };
		constexpr sf::Vector2f QUIT_BTN_SIZE = { 0.3f, 0.1f };

		constexpr sf::Vector2f START_BTN_POS = { 0.65f, 0.65f };
		constexpr sf::Vector2f START_BTN_SIZE = { 0.3f, 0.1f };

		constexpr sf::Vector2f SETTINGS_BTN_POS = { 0.5f, 0.5f };
		constexpr sf::Vector2f SETTINGS_BTN_SIZE = { 0.1f, 0.1f };

		// Texts
		constexpr sf::Vector2f START_TEXT_POS = { 0.5f, 0.35f };
	}
	namespace Stage {
		// Buttons
		constexpr  sf::Vector2f PAUSE_BTN_POS = { 0.15f,0.25f };
		constexpr  sf::Vector2f PAUSE_BTN_SIZE = { 0.1f,0.1f };

		constexpr  sf::Vector2f CANNON_BTN_POS = { 0.8f,0.8f };
		constexpr  sf::Vector2f CANNON_BTN_SIZE = { 0.1f,0.1f };

		constexpr  sf::Vector2f BAG_BTN_POS = { 0.12f,0.85f };
		constexpr  sf::Vector2f BAG_BTN_SIZE = { 0.1f,0.1f };

		constexpr  sf::Vector2f CLOSE_GAME_BTN_POS = { 0.4f, 0.7f };
		constexpr  sf::Vector2f CLOSE_GAME_BTN_SIZE = { 0.1f, 0.1f };

		constexpr  sf::Vector2f CLOSE_MENU_BTN_POS = { .6f, .7f };
		constexpr  sf::Vector2f CLOSE_MENU_BTN_SIZE = { 0.1f, 0.1f };

		// Texts
		constexpr  sf::Vector2f PARTS_TEXT_POS = { 0.1f,0.1f };
		constexpr  sf::Vector2f CHALLENGES_TEXT_POS = { 0.7f,0.8f };
		constexpr  sf::Vector2f BAG_COST_TEXT_POS = { 0.05f,0.9f };
		constexpr  sf::Vector2f PAUSE_TEXT_POS = { 0.5f, 0.35f };

		// Sprites
		constexpr  sf::Vector2f PAUSE_MENU_POS = { 0.5f,0.5f };
		constexpr  sf::Vector2f PAUSE_MENU_SIZE = { 0.2f,0.2f };

		constexpr  sf::Vector2f FIRST_UNIT_SLOT_POS = { 0.278f, 0.84375f };
		constexpr  sf::Vector2f UNIT_SLOT_INCREMENT = { 0.089f, 0.0625f };
	}
}
