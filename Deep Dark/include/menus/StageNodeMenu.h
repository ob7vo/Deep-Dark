#pragma once
#include "Menu.h"

constexpr float TRANSITION_DURATION = 0.35f;

// The side menu that opens up when you click a stage on the Stage Select screen
struct StageNodeMenu : Menu<UI::StageSelect::StageNode::BTN_COUNT> {
	sf::Sprite menuSprite = sf::Sprite(defTex);
    sf::Text stageNameText = sf::Text(baseFont);

    explicit StageNodeMenu(Camera& cam);
    ~StageNodeMenu() final = default;

    void draw() final;
    void reset_positions() final;

    void slide_menu_to_point(float t);
    void set_stage_name_text(int stageID);

    inline Button& closeBtn() { return buttonManager.buttons[static_cast<int>(UI::StageSelect::StageNode::SNM_ButtonIndex::CLOSE)]; }
    inline Button& startStageBtn() { return buttonManager.buttons[static_cast<int>(UI::StageSelect::StageNode::SNM_ButtonIndex::START_STAGE)]; }
    inline Button& enterArmoryBtn() { return buttonManager.buttons[static_cast<int>(UI::StageSelect::StageNode::SNM_ButtonIndex::ENTER_ARMORY)]; }
};