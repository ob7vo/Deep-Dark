#pragma once
#include "Menu.h"

const int STAGE_SELECT_BTNS = 4;
const int STAGES = 3;

struct StageSelect : public Menu<STAGE_SELECT_BTNS> {
      sf::Text selectStageText = sf::Text(baseFont);

      StageSelect(Camera& cam);
      ~StageSelect() = default;

      void draw() override;
      void check_mouse_hover() override;
      void register_click() override;
      void reset_positions() override;

      inline Button& stageBtn(int stage) {
          return buttonManager.buttons[stage];
      }
      inline Button& returnBtn() {return buttonManager.buttons[STAGES];}
}
