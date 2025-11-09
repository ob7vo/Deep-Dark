#pragma once
#include "Menu.h"
#include "Slider.h"

const int ARMORY_BTNS = 3;

struct ArmoryMenu : public Menu<ARMORY_BTNS> {
    sf::Text armoryText = sf::Text(baseFont);
    // first = unit ID. second = unit form/gear
    
    Slider unitSlider = {};
    std::vector<std::pair<int,int>> equippedUnits = {};
    std::vector<int> equippedCores = {};

    ArmoryMenu(Camera& cam);
    ~ArmoryMenu() = default;

    void draw() override;
    void check_mouse_hover() override;
    bool on_mouse_press(bool isM1) override;
    bool on_mouse_release(bool isM1) override;
    void reset_positions() override;
    void update(float deltaTime) override;

    inline Slider& slider() { return unitSlider; }
};