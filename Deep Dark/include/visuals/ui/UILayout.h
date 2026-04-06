 #pragma once
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>
#include <array>
#include <UnitConfigConstants.h>
#include <StageConfig.h>

namespace UI 
{
constexpr sf::Vector2f ZERO = { 0.f, 0.f };
 
namespace Colors {
	constexpr sf::Color GREY(128, 128, 128);

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
	constexpr int BTN_COUNT = UnitConfig::TOTAL_PLAYER_UNITS + 2;

	constexpr sf::Vector2f FIRST_EQUIP_SLOT_POS = { 0.225f, 0.15f };
	constexpr sf::Vector2f EQUIP_SLOT_SPACING = { 0.15f, 0.1f };
	constexpr sf::Vector2f EQUIP_SLOT_SCALE = { 0.08f, 0.08f };
	constexpr int EQUIP_SLOTS = 10;

	constexpr sf::Vector2f FIRST_INVENTORY_SLOT_POS = { 0.1f, 0.8f };
	constexpr sf::Vector2f INVENTORY_SLOT_SPACING = { 0.1f, 0.f };
	constexpr sf::Vector2f INVENTORY_SLOT_SCALE = { 0.08f, 0.08f };

	constexpr sf::Vector2f DRAG_SLOT_ORIGIN = { 0.5f, 0.5f };
	constexpr float DRAG_SLOT_OPACITY = 0.65f;

	constexpr sf::Vector2f SLIDER_POS = { 0.1f, 0.6f };
	constexpr sf::Vector2f SLIDER_SIZE = { 0.2f, 0.1f };
	constexpr std::pair<float,float> SLIDER_RANGE = { 0.1f, 0.9f };

	constexpr sf::Vector2f RETURN_BTN_POS = { 0.9f, 0.1f };
	constexpr sf::Vector2f RETURN_BTN_SIZE = { 0.08f, 0.08f };

	constexpr sf::Vector2f STAGE_SET_BTN_POS = { 0.1f, 0.1f };
	constexpr sf::Vector2f STAGE_SET_BTN_SIZE = { 0.08f, 0.08f };

	enum class ButtonIndex {
		RETURN = UnitConfig::TOTAL_PLAYER_UNITS,
		STAGE_SET = BTN_COUNT - 1
	};
	// 0.089f, 0.0625f

	namespace StagePreview {
		constexpr int BTN_COUNT = 3;

		constexpr sf::Vector2f BACKGROUND_POS = { .5f, 0.5f };
		constexpr sf::Vector2f BACKGROUND_SIZE = { .5f, 0.5f };

		constexpr sf::Vector2f TEXT_POS = { .5f, 0.25f };
		constexpr float TEXT_HEIGHT = 0.035f;

		constexpr sf::Vector2f ENEMY_UNITS_CENTER_POS = { 0.5f, 0.325f };
		constexpr sf::Vector2f ENEMY_UNITS_SPACING = { 0.05f, 0.f };
		constexpr sf::Vector2f ENEMY_UNITS_SIZE = { 0.08f, 0.08f };

		constexpr sf::Vector2f FIRST_UNIT_SLOT_POS = { 0.275f, 0.4f };
		constexpr sf::Vector2f UNIT_SLOT_INCREMENT = { 0.065f, 0.06f };
		constexpr sf::Vector2f UNIT_SLOT_SCALE = { 0.05f, 0.05f };

		constexpr sf::Vector2f START_BTN_POS = { .3f, .5f };
		constexpr sf::Vector2f START_BTN_SIZE = { .06f, 0.06f };

		constexpr sf::Vector2f EXIT_BTN_POS = { .6f, .5f };
		constexpr sf::Vector2f EXIT_BTN_SIZE = { .06f, .06f };

		constexpr sf::Vector2f CLOSE_MENU_BTN_POS = { 0.45f, 0.55f };
		constexpr sf::Vector2f CLOSE_MENU_BTN_SIZE = { 0.06f, 0.06f };

		enum class SP_ButtonIndex {
			START_STAGE,
			CLOSE,
			EXIT_STAGE
		};
	} // namespace StageSet
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
	constexpr float START_TEXT_SIZE = 0.005f;

	enum class ButtonIndex {
		START,
		QUIT,
		SETTINGS
	};
} // StartMenu

namespace StageUI {
	constexpr int BTN_COUNT = 3;

	// How far UI slides offscreen during transitions
	constexpr float DIST_OFFSCREEN = 0.2f; //norm value of screen position

	// Buttons
	constexpr sf::Vector2f PAUSE_BTN_POS = { 0.075f,0.125f };
	constexpr sf::Vector2f PAUSE_BTN_SIZE = { 0.1f,0.1f };

	constexpr sf::Vector2f FIRE_CANNON_BTN_POS = { 0.8f,0.8f };
	constexpr sf::Vector2f FIRE_CANNON_BTN_SIZE = { 0.1f,0.1f };

	constexpr sf::Vector2f UPGRADE_BAG_BTN_POS = { 0.12f,0.85f };
	constexpr sf::Vector2f UPGRADE_BAG_BTN_SIZE = { 0.1f,0.1f };

	// Texts
	constexpr sf::Vector2f PARTS_TEXT_POS = { 0.1f,0.1f };
	constexpr sf::Vector2f CLEARED_CHALLENGES_TEXT_POS = { 0.7f,0.8f };
	constexpr sf::Vector2f BAG_UPGRADE_COST_TEXT_POS = { 0.05f,0.9f };
	constexpr float TEXT_SIZE = 0.03f;

	// Sprites
	constexpr sf::Vector2f FIRST_UNIT_SLOT_POS = { 0.278f, 0.84375f };
	constexpr sf::Vector2f UNIT_SLOT_SPACING = { 0.089f, 0.0625f };

	enum class ButtonIndex {
		PAUSE,
		UPGRADE_BAG,
		FIRE_CANNON
	};

	namespace PauseMenu {
		constexpr int BTN_COUNT = 2;

		constexpr sf::Vector2f CLOSE_GAME_BTN_POS = { 0.4f, 0.7f };
		constexpr sf::Vector2f CLOSE_GAME_BTN_SIZE = { 0.1f, 0.1f };

		constexpr sf::Vector2f CLOSE_MENU_BTN_POS = { .6f, .7f };
		constexpr sf::Vector2f CLOSE_MENU_BTN_SIZE = { 0.1f, 0.1f };

		constexpr sf::Vector2f PAUSE_MENU_POS = { 0.5f,0.5f };
		constexpr sf::Vector2f PAUSE_MENU_SIZE = { 0.2f,0.2f };
		constexpr sf::Vector2f PAUSE_MENU_TEXT_POS = { 0.5f, 0.35f };

		enum class ButtonIndex {
			CLOSE_GAME,
			CLOSE_MENU
		};
	} // namespace PauseMenu
	namespace ResultsScreen {
		constexpr int BTN_COUNT = 3;

		constexpr sf::Vector2f MENU_POS = { 0.5f, 0.5f };
		constexpr sf::Vector2f MENU_SIZE = { 0.4f, 0.3f };

		constexpr sf::Vector2f VICTORY_TEXT_POS = { 0.5f, 0.25f };
		constexpr float VICTORY_TEXT_SIZE = 0.004f;

		constexpr sf::Vector2f QUIT_BTN_POS_LEFT = { 0.35f, 0.55f }; // When "Next Stage" buttn is present
		constexpr sf::Vector2f QUIT_BTN_POS_CENTER = { 0.5f, 0.55f }; // When this button is alone
		constexpr sf::Vector2f SECOND_STAGE_BTN_POS = { 0.65f, 0.55f };
		constexpr sf::Vector2f STAGE_BTNS_SIZE = { 0.13f, 0.13f };
		
		enum class VS_ButtonIndex {
			QUIT_STAGE = 0,
			NEXT_STAGE_SET,
			RESTART_STAGE,
		};
	} // namespace VictoryScreen
} //namespace Stage

namespace StageSelect {
	constexpr int BTN_COUNT = StageConfig::TOTAL_STAGES + 1;

	constexpr float DIST_OFFSCREEN = 0.2f;

	constexpr sf::Vector2f RETURN_BTN_POS = { .1f,.1f };
	constexpr sf::Vector2f RETURN_BTN_SIZE = { .1f, .1f };

	constexpr sf::Vector2f STAGE_NODE_SIZE = { 0.12f, 0.12f };
	constexpr std::array<sf::Vector2f, 3> STAGE_NODE_POS_ARR = { {
		{350.f,350.f},
		{1400.f, 250.f},
		{350.f, 1400.f}
	} }; 

	const enum ButtonIndex : int {
		LAST_STAGE = 3, // Button Index for the last stage
		RETURN = 3,
	};

	namespace StageNode {
		constexpr int BTN_COUNT = 3 + StageConfig::MAX_PHASES;

		constexpr float TRANSITION_SLIDE_DISTANCE = 0.5f;

		constexpr sf::Vector2f MENU_POS = { 0.75f, 0.5f };
		constexpr sf::Vector2f MENU_SIZE = { 0.5f, 1.f };

		constexpr sf::Vector2f STAGE_NAME_TEXT_POS = { .75f, .2f };
		constexpr float STAGE_NAME_TEXT_SIZE = 0.05f;

		constexpr sf::Vector2f CLOSE_BTN_POS = { .6f,.1f };
		constexpr sf::Vector2f CLOSE_BTN_SIZE = { .08f, .08f };

		constexpr sf::Vector2f START_STAGE_BTN_POS = { .65f,.5f };
		constexpr sf::Vector2f START_STAGE_BTN_SIZE = { .1f, .1f };

		constexpr sf::Vector2f PRACTICE_SET_BTNS_CENTER_POS = { 0.75f, 0.8f };
		constexpr sf::Vector2f PRACTICE_SET_BTNS_SPACING = { 0.04f, 0.f };
		constexpr sf::Vector2f PRACTICE_SET_BTNS_SIZE = { 0.08f, 0.08f };

		constexpr sf::Vector2f ARMORY_BTN_POS = { .85f,.5f };
		constexpr sf::Vector2f ARMORY_BTN_SIZE = { .1f, .1f };

		const enum SNM_ButtonIndex : int {
			CLOSE = 0, // Button Index for the last stage
			START_STAGE = 1,
			ENTER_ARMORY = 2,
			PRACTICE_SETS = 3,
			END_OF_PRACTICE_SETS = 3 + StageConfig::MAX_PHASES - 1
		};
	} // namespace StageNode
} // namespace StageSelect

namespace Workshop {
	constexpr int BTN_COUNT = 5 + UnitConfig::TOTAL_ANIM_COUNT;

	constexpr sf::Vector2f RETURN_BTN_POS = { 0.1f, 0.1f };
	constexpr sf::Vector2f RETURN_BTN_SIZE = { 0.05f, 0.05f };

	constexpr sf::Vector2f STAT_ICON_POS = { 0.2f, 0.75f };
	constexpr sf::Vector2f STAT_ICON_SIZE = { 0.07f, 0.07f };
	constexpr sf::Vector2f STAT_ICON_SPACING = { 0.15f, 0.f };
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

	constexpr sf::Vector2f SCRAP_PARTS_ICON_POS = { 0.8f, 0.085f };
	constexpr sf::Vector2f SCRAP_PARTS_ICON_SIZE = { 0.05f, 0.05f };
	constexpr sf::Vector2f SCRAP_PARTS_TEXT_POS = { 0.85f, 0.085f };
	constexpr float SCRAP_PARTS_TEXT_SIZE = 0.03f;

	constexpr sf::Vector2f UPGRADE_UNIT_BTN_POS = { 0.45f, 0.325f };
	constexpr sf::Vector2f UPGRADE_UNIT_BTN_SIZE = { 0.04f, 0.04f };
	constexpr sf::Vector2f UPGRADE_COST_TEXT_POS = { 0.5125f, 0.325f };
	constexpr float UPGRADE_COST_TEXT_SIZE = 0.03f;

	enum class ButtonIndex {
		RETURN = 0,
		PAUSE,
		SWITCH_GEAR,
		SPEED_UP,
		FIRST_ANIMATION = 4,
		LAST_ANIMATION = 13,
		UPGRADE_UNIT = 14,
		COUNT,
	};
} // namespace Workshop

namespace Creator {
	namespace Unit {
		constexpr int BTN_COUNT = 3;
		constexpr int CLICKABLE_TEXT_COUNT = 12;

		/*"hp": 30,
    "knockbacks": 3,
    "dmg": 20,
    "speed": 15,
    "attack_time": 0.5,
    "sight_range": 50.0,
    "attack_range": [ 0, 50 ],
    "single_target": true,
    "parts_cost": 5,
    "recharge_timer": 2.25
  },
  "hurtbox": {
    "width": 30.0,
    "height": 30.0*/
		// Hit Data (dmg, attack range, attack reach) are in vectors
		
		enum class TextIndex {
			STATS_START = 0,
			//ints
			HP = 0,
			KNOCKBACKS = 1,
			PARTS_COST = 2,
			// floats
			SPEED = 3,
			SIGHT_RANGE = 4,
			ATTACK_COOLDOWN = 5,
			RECHARGE_TIMER = 6,
			// pair<int,int>
			LANE_SIGHT = 7,
			// bool
			SINGLE_TARGET = 8,
			STATS_END = 8,
			UNIT_ID = 9,
			UNIT_GEAR = 10,
			UNIT_NAME = 11
		};
	};
	namespace Stage {
		constexpr int BTN_COUNT = 4;
	};
}
} // namespace UI
