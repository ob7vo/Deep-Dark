#pragma once
#include "Menu.h"
#include "StageNodeMenu.h"

struct StageSelect : public Menu<UI::StageSelect::BTN_COUNT> {
    StageNodeMenu stageNodeMenu;
    bool isTransitioning = false;
    float transitionTime = 0.f;
    float zoomLevelBeforeTransition = 1.f;
    sf::Vector2f camStartingPos;

    sf::Text selectStageText = sf::Text(baseFont);

    /// <summary>
    /// The stage the user has clicked on to view. Needed for creating ArmoryMenu
    /// Since I haven't yet made the Stage View menu, this will be the last stage hoveredd on
    /// </summary>
    int selectedStage = 0;

    explicit StageSelect(Camera& cam);
    ~StageSelect() final = default;

    void update(float deltaTime) final;
    void draw() final;
    void check_mouse_hover() final;
    bool on_mouse_press(bool isM1) final;
    void reset_positions() final;

    void on_enter() final;

    void start_stage_node_menu_transition();
    void transition_stage_node_menu(float deltaTime);
    void slide_ui_to_point(float t);

    inline Button& stageNodeBtn(int stage) {
        return buttonManager.buttons[stage];
    }
    inline Button& returnBtn() { return buttonManager.buttons[static_cast<int>(UI::StageSelect::ButtonIndex::RETURN)]; }

    inline bool zoomedInOnNode() { return stageNodeMenu.clickable; }
};
