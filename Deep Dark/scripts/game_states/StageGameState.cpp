#include "StageGameState.h"

StageGameState::StageGameState(Camera& cam) : stageUI(cam), loadout(cam),
stageManager(cam, stageUI, loadout){
	stageUI.stageManager = &stageManager;
}
void StageGameState::update_ui(float deltaTime) {
	for (int i = 0; i < loadout.filledSlots; i++)
		loadout.slots[i].cooldown -= deltaTime;
	stageUI.partsCountText.setString(std::format("#{}/{}", stageManager.parts, stageManager.bagCap));
}
void StageGameState::update(float deltaTime) {
	stageUI.check_mouse_hover();
	if (stageUI.paused) return;
	stageManager.update_game_ticks(deltaTime);
	update_ui(deltaTime);
}
void StageGameState::render(Camera& cam) {
	stageManager.draw(cam.get_window());
	loadout.draw_slots(cam, stageManager.parts);
	stageUI.draw();
}
void StageGameState::handle_events(sf::Event event) {
	stageManager.handle_events(event);
	if (event.is<sf::Event::MouseButtonPressed>())
		stageUI.register_click();
}
void StageGameState::on_enter(OnStateEnterData* enterData) {
	if (StageEnterData* stageData = dynamic_cast<StageEnterData*>(enterData)) {
		loadout.create_loadout(stageData->loadoutSlots);
		loadout.set_slot_positions(stageData->cam);
		stageManager.create_stage(stageData->stageJson);
	}
	else {
		std::cout << "enterData is not of type [StageEnterData]" << std::endl;
		return;
	}
}
void StageGameState::on_exit() {
	stageManager.unload_stage();
	loadout.slotTextures = {};
}
GameState::Type StageGameState::get_next_state() {
	return GameState::Type::STAGE;
}