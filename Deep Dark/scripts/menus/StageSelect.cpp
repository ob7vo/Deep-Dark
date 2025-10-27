#include "StageSelect.h"
#include "UILayout.h"

using namespace UI::StageSelect;

StageSelect::StageSelect(Camera& cam) : Menu(cam){
  	returnBtn().set_new_params(RETURN_BTN_POS, RETURN_BTN_SIZE,
  	std::string texPath = "sprites/ui/stage_select/stage_node.png";
	for (int i = 0; i < STAGES; i++) 
        stageBtn(i).set_new_params({1.f,1.f}, STAGE_NODE_SIZE, texPath, cam);

	std::string texPath2 = "sprites/ui/stage_select/return_btn.png";
	returnBtn().set_new_params(RETURN_BTN_POS, RETURN_BTN_SIZE, texPath2, cam(;

	reset_positions();
}

void StageSelect::reset_positions() {
    for (int i = 0; i < STAGES; i++) 
	       stageBtn(i).sprite.setPosition(STAGE_NODE_POS_ARR[i]);

	returnBtn().sprite.setPosition(RETURN_BTN_POS);
}
