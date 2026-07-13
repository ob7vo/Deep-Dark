#pragma once
#include <Window.h>

struct UnitInspectorWindowLayout {

    	//constexpr int BTN_COUNT = 4 + UnitConfig::TOTAL_ANIM_COUNT;


	//constexpr float STAT_TEXT_SIZE = 0.025f;

	// Slider and Borders
	sf::Vector2f SLIDER_POS = { 0.2f, 0.65f };
	sf::Vector2f SLIDER_AXIS_BOUNDS = { 0.2f, 0.8f };

	// Unit Animation
	sf::Vector2f UNIT_POS = { 0.5f, 0.45f };

	sf::Vector2f UNIT_NAME_TEXT_POS = { 0.5f,0.1f };
	sf::Vector2f UNIT_DESC_TEXT_POS = { 0.5f, 0.15f };
	//constexpr float UNIT_TEXT_SIZE = 0.03f;
	
	sf::Vector2f UNIT_PAUSE_BTN_POS = { 0.35f, 0.3f };
	sf::Vector2f UNIT_PAUSE_BTN_SIZE = { 0.05f, 0.05f };

	sf::Vector2f SCRAP_PARTS_ICON_POS = { 0.8f, 0.085f };
	sf::Vector2f SCRAP_PARTS_ICON_SIZE = { 0.05f, 0.05f };
	sf::Vector2f SCRAP_PARTS_TEXT_POS = { 0.85f, 0.085f };
	//float SCRAP_PARTS_TEXT_SIZE = 0.03f;

	sf::Vector2f UPGRADE_UNIT_BTN_POS = { 0.45f, 0.325f };
	sf::Vector2f UPGRADE_UNIT_BTN_SIZE = { 0.04f, 0.04f };
	sf::Vector2f UPGRADE_COST_TEXT_POS = { 0.5125f, 0.325f };
};

class UnitInspectorWindow : public Window<UI::ArmoryMenu::BTN_COUNT>
{
    UnitInspectorWindowLayout layout;
public:
    explicit UnitInspectorWindow(Camera& cam) : Window<UI::ArmoryMenu::BTN_COUNT>(cam) {};
    ~UnitInspectorWindow() final = default;

    void draw() override;
    void reset_positions() override;
};