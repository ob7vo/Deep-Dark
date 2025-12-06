#pragma once
#include "Menu.h"

const int STAGES = 3;

struct StageSelect : public Menu<UI::StageSelect::BTN_COUNT> {
    sf::Text selectStageText = sf::Text(baseFont);

    /// <summary>
    /// The stage the user has clicked on to view. Needed for creating ArmoryMenu
    /// Since I havent yet made the Stage View menu, this will be the last stage hoveredd on
    /// </summary>
    int selectedStage = 0;

    explicit StageSelect(Camera& cam);
    ~StageSelect() final = default;

    void draw() final;
    void check_mouse_hover() final;
    bool on_mouse_press(bool isM1) final;
    void reset_positions() final;

    inline Button& stageNodeBtn(int stage) {
        return buttonManager.buttons[stage];
    }
    inline Button& returnBtn() { return buttonManager.buttons[static_cast<int>(UI::StageSelect::ButtonIndex::RETURN)]; }
    inline Button& enterArmoryBtn() { return buttonManager.buttons[static_cast<int>(UI::StageSelect::ButtonIndex::ENTER_ARMORY)]; }
};
