#pragma once
#include <SFML/System/Vector2.hpp>
#include <array>

namespace UI 
{
constexpr sf::Vector2f ZERO = { 0.f, 0.f };
 
namespace Colors {
	constexpr sf::Color BLACK_TRANSPARENT(0, 0, 0, 128);
	constexpr sf::Color RED_TRANSPARENT(255, 0, 0, 128);

	constexpr std::array<sf::Color, 4> UNIT_HITBOX_COLORS = {
		sf::Color(240, 48, 38, 128), // red
		sf::Color(232, 137, 35, 128), // orange
		sf::Color(247, 227, 47, 128), // yellow
		sf::Color(240, 48, 38, 128)
	};
} // namespace Colors

//MENUS
namespace ArmoryMenu {
	constexpr int BTN_COUNT = 4;

	constexpr sf::Vector2f FIRST_SLOT_POS = { 0.225f, 0.15f };
	constexpr sf::Vector2f SLOT_INCREMENT = { 0.15f, 0.1f };
	constexpr sf::Vector2f SLOT_SCALE = { 0.06f, 0.06f };

	constexpr sf::Vector2f SLIDER_SLOT_POS = { 0.1f, 0.8f };
	constexpr sf::Vector2f SLIDER_SLOT_INCREMENT = { 0.1f, 0.f };
	constexpr sf::Vector2f SLIDER_SLOT_SCLAE = { 0.08f, 0.08f };

	constexpr sf::Vector2f DRAG_SLOT_ORIGIN = { 0.5f, 0.5f };
	constexpr float DRAG_SLOT_OPACITY = 0.65f;

	constexpr sf::Vector2f SLIDER_POS = { 0.1f, 0.6f };
	constexpr sf::Vector2f SLIDER_SIZE = { 0.2f, 0.1f };
	constexpr sf::Vector2f SLIDER_AXIS_BOUNDS = { 0.1f, 0.9f };

	constexpr sf::Vector2f RETURN_BTN_POS = { 0.9f, 0.1f };
	constexpr sf::Vector2f RETURN_BTN_SIZE = { 0.08f, 0.08f };
	// 0.089f, 0.0625f
} // namespace ArmoryMenu

namespace StartMenu {
	constexpr int BTN_COUNT = 3;
	// Buttons
	constexpr sf::Vector2f QUIT_BTN_POS = { 0.3f, 0.65f };
	constexpr sf::Vector2f QUIT_BTN_SIZE = { 0.3f, 0.1f };

	constexpr sf::Vector2f START_BTN_POS = { 0.65f, 0.65f };
	constexpr sf::Vector2f START_BTN_SIZE = { 0.3f, 0.1f };

	constexpr sf::Vector2f SETTINGS_BTN_POS = { 0.5f, 0.5f };
	constexpr sf::Vector2f SETTINGS_BTN_SIZE = { 0.1f, 0.1f };

	// Texts
	constexpr sf::Vector2f START_TEXT_POS = { 0.5f, 0.35f };
} // StartMenu

namespace StageUI {
	constexpr int UI_BTN_COUNT = 3;
	constexpr int PAUSE_MENU_BTN_COUNT = 2;
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
} //namespace Stage

namespace StageSelect {
	constexpr int BTN_COUNT = 5;
	constexpr sf::Vector2f RETURN_BTN_POS = { .2f,.2f };
	constexpr sf::Vector2f RETURN_BTN_SIZE = { .1f, .1f };
	
	constexpr sf::Vector2f ARMORY_BTN_POS = { .85f,.7f };
	constexpr sf::Vector2f ARMORY_BTN_SIZE = { .1f, .1f };

	constexpr sf::Vector2f STAGE_NODE_SIZE = { 0.12f, 0.12f };
	constexpr std::array<sf::Vector2f, 3> STAGE_NODE_POS_ARR = { {
		{350.f,350.f},
		{1400.f, 250.f},
		{350.f, 1400.f}
	} };
} // namespace StageSelect

namespace StageSet {
	constexpr int BTN_COUNT = 3;

	constexpr sf::Vector2f BACKGROUND_POS = { .5f, 0.4f };
	constexpr sf::Vector2f BACKGROUND_SIZE = { .3f, 0.125f };
	constexpr float TEXT_HEIGHT = 0.035f;

	constexpr sf::Vector2f FIRST_UNIT_SLOT_POS = { 0.225f, 0.15f };
	constexpr sf::Vector2f UNIT_SLOT_INCREMENT = { 0.15f, 0.1f };
	constexpr sf::Vector2f UNIT_SLOT_SCALE = { 0.06f, 0.06f };
} // namespace StageSet

namespace Workshop {
	constexpr int BTN_COUNT = 10;

	constexpr sf::Vector2f RETURN_BTN_POS = { 0.1f, 0.1f };
	constexpr sf::Vector2f RETURN_BTN_SIZE = { 0.05f, 0.05f };

	constexpr sf::Vector2f STAT_ICON_POS = { 0.2f, 0.75f };
	constexpr sf::Vector2f STAT_ICON_SIZE = { 0.07f, 0.07f };
	constexpr sf::Vector2f STAT_ICON_INCREMENT = { 0.15f, 0.f };
	constexpr sf::Vector2f STAT_TEXT_OFFSET = { 0.03f, 0.f };
	constexpr float STAT_TEXT_SIZE = 0.025f;

	// Slider and Borders
	constexpr sf::Vector2f SLIDER_POS = { 0.2f, 0.65f };
	constexpr sf::Vector2f SLIDER_AXIS_BOUNDS = { 0.2f, 0.8f };

	// Unit Animation
	constexpr sf::Vector2f UNIT_POS = { 0.5f, 0.45f };
	constexpr sf::Vector2f UNIT_HURT_BOX_SIZE = { 0.05f, 0.03f };
	constexpr std::array<float, 4> UNIT_HITBOX_HEIGHTS = {30.f, 45.f, 60.f, 75.f};

	constexpr sf::Vector2f UNIT_NAME_TEXT_POS = { 0.5f,0.1f };
	constexpr sf::Vector2f UNIT_DESC_TEXT_POS = { 0.5f, 0.15f };
	constexpr float UNIT_TEXT_SIZE = 0.03f;

	constexpr sf::Vector2f UNIT_PAUSE_BTN_POS = { 0.35f, 0.3f };
	constexpr sf::Vector2f UNIT_PAUSE_BTN_SIZE = { 0.05f, 0.05f };
	constexpr sf::Vector2f UNIT_SWITCH_GEAR_BTN_POS = { 0.35f, 0.4f };
	constexpr sf::Vector2f UNIT_SWITCH_GEAR_BTN_SIZE = { 0.05f, 0.05f };
	constexpr sf::Vector2f UNIT_SPEED_BTN_POS = { 0.35f, 0.5f };
	constexpr sf::Vector2f UNIT_SPEED_BTN_SIZE = { 0.05f, 0.05f };

	constexpr sf::Vector2f UNIT_ANIMATION_BTN_POS = { 0.65f, 0.25f };
	constexpr sf::Vector2f UNIT_ANIMATION_BTN_INCREMENT = { 0.f, 0.075f };
	constexpr sf::Vector2f UNIT_ANIMATION_BTN_SIZE = { 0.05f, 0.05f };
} // namespace Workshop

} // namespace UI
