#include "StageSelect.h"
#include "UILayout.h"

using namespace UI::StageSelect;

StageSelect::StageSelect(Camera& cam) : Menu(cam){
  
    	std::string texPath = "sprites/ui/stage_select/stage_node.png";
      for (int i = 0; i < STAGES; i++) {
	        //stageBtn(i).set_new_params(STAGE_NODE_POS_ARR[i], STAGE_NODE_SIZE, texPath, cam);
      }
}

void StageSelect::reset_positions() {
    for (int i = 0; i < STAGES; i++) {
	       // stageBtn(i).sprite.setPosition(STAGE_NODE_POS_ARR[i], STAGE_NODE_SIZE, texPath, cam);
      }
}
