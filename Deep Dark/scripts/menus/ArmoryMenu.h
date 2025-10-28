#pragma once
#include "Menu.h"

const int ARMORY_BTNS = 10;

struct ArmoryMenu : public Menu<ARMORY_BTNS> {
    sf::Text armoryText = sf::Text(baseFont);
    std::vector<std::string> equippedUnits = {};

    ArmoryMenu(Camera& cam);
    ~ArmoryMenu() = default;

    void draw() override;
    void check_mouse_hover() override;
    void register_click() override;
    void reset_positions() override;
};