#pragma once
#include "Menu.h"

const int STAGES = 3;

struct StageSelect : public Menu<UI::StageSelect::BTN_COUNT> {
    sf::Text selectStageText = sf::Text(baseFont);

    explicit StageSelect(Camera& cam);
    ~StageSelect() final = default;

    void draw() final;
    void check_mouse_hover() final;
    bool on_mouse_press(bool isM1) final;
    void reset_positions() final;

    inline Button& stageNodeBtn(int stage) {
        return buttonManager.buttons[stage];
    }
    inline Button& returnBtn() { return buttonManager.buttons[STAGES]; }
    inline Button& armoryBtn() { return buttonManager.buttons[STAGES + 1]; }
};
