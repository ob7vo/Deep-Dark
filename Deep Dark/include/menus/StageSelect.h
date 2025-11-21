#pragma once
#include "Menu.h"

const int STAGES = 3;
const int STAGE_SELECT_BTNS = 2 + STAGES;

struct StageSelect : public Menu<STAGE_SELECT_BTNS> {
    sf::Text selectStageText = sf::Text(baseFont);

    StageSelect(Camera& cam);
    ~StageSelect() = default;

    void draw() override;
    void check_mouse_hover() override;
    bool on_mouse_press(bool isM1) override;
    void reset_positions() override;

    inline Button& stageBtn(int stage) {
        return buttonManager.buttons[stage];
    }
    inline Button& returnBtn() { return buttonManager.buttons[STAGES]; }
    inline Button& armoryBtn() { return buttonManager.buttons[STAGES + 1]; }
};
