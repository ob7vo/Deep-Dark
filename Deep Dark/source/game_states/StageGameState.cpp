#include "StageGameState.h"
#include "PreparationState.h"
#include "ArmoryMenu.h"

StageState::StageState(Camera& cam) : stageUI(cam), loadout(cam),
stageManager(cam, stageUI, loadout), GameState(cam){
	stageUI.reset_positions();

	stageUI.stageManager = &stageManager;
	stageUI.pauseMenu.closeGameBtn().onClick = [this](bool m1) { if (m1) quit_stage(); };
}
StageEnterData::StageEnterData(const nlohmann::json& stageJson, ArmoryMenu& armory) : 
	stageJson(stageJson), slots(armory.slots), OnStateEnterData(GameState::Type::STAGE) {}

void StageState::update_ui(float deltaTime) {
	for (int i = 0; i < loadout.filledSlots; i++)
		loadout.slots[i].cooldown -= deltaTime;
	stageUI.partsCountText.setString(std::format("#{}/{}", stageManager.parts, stageManager.bagCap));
}
void StageState::update(float deltaTime) {
	stageUI.check_mouse_hover();
	if (stageUI.paused) return;
	stageManager.update_game_ticks(deltaTime);
	update_ui(deltaTime);
}
void StageState::render() {
	stageManager.draw(cam.get_window());
	loadout.draw_slots(cam, stageManager.parts);
	stageUI.draw();
}
void StageState::handle_events(sf::Event event) {
	stageManager.handle_events(event);
	if (auto click = event.getIf<sf::Event::MouseButtonPressed>())
		clicked = stageUI.on_mouse_press(is_M1(click));
}

void StageState::quit_stage() {
	MenuType cur = MenuType::STAGE_SELECT;
	MenuType prev = MenuType::HOME_BASE;

	nextStateEnterData = std::make_unique<PrepEnterData>(cur, prev);
	readyToEndState = true;
}
void StageState::on_enter(OnStateEnterData* enterData) {
	if (auto stageData = dynamic_cast<StageEnterData*>(enterData)) {
		loadout.create_loadout(stageData->slots);
		loadout.set_slot_positions(cam);
		stageManager.create_stage(stageData->stageJson);

		cam.reset();
		cam.change_lock(false);

		std::cout << loadout.slotTextures.size() << "slot textures\n";
	}
	else {
		std::cout << "enterData is not of type [StageEnterData]" << std::endl;
		return;
	}
}
void StageState::on_exit() {
	std::cout << "unloading stage" << std::endl;
	stageManager.unload_stage();
	loadout.slotTextures = {};

	stageUI.paused = false;
	for (int i = 0; i < STAGE_UI_BUTTONS; i++)
		stageUI.buttonManager.buttons[i].sprite.setColor(sf::Color::White);
}
