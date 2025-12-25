#include "pch.h"
#include "StageGameState.h"
#include "PreparationState.h"
#include "ArmoryMenu.h"

StageState::StageState(Camera& cam) : 
	GameState(cam), 
	stageUI(cam), 
	loadout(cam),
	stageManager(cam, stageUI, loadout)
{
	stageUI.reset_positions();

	stageUI.stageManager = &stageManager;
	stageUI.pauseMenu.closeGameBtn().onClick = [this](bool m1) { if (m1) quit_stage(); };
}
StageEnterData::StageEnterData(const std::string& path, int set, 
	const std::array<ArmorySlot, 10>& slots) :
	OnStateEnterData(GameState::Type::STAGE), 
	stageJsonPath(path), 
	stageSet(set), 
	slots(slots) {}

void StageState::update_ui(float deltaTime) {
	for (int i = 0; i < loadout.filledSlots; i++)
		loadout.slots[i].cooldown -= deltaTime;

	stageUI.partsCountText.setString(std::format("#{}/{}", stageManager.wallet.parts, stageManager.wallet.partsCap));
}
void StageState::update(float deltaTime) {
	stageUI.check_mouse_hover();
	if (stageUI.paused) return;

	stageManager.tick(deltaTime);
	update_ui(deltaTime);
}
void StageState::render() {
	stageManager.draw(cam.get_window());
	loadout.draw_slots(cam, stageManager.wallet.parts);
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

	curStageSet = 0;
	stageSetCount = 1;
	stageManager.unload();

	nextStateEnterData = std::make_unique<PrepEnterData>(cur, prev);
	readyToEndState = true;
}
void StageState::end_current_stage_set() {
	if (++curStageSet >= stageSetCount) quit_stage(); // no logic for winning at the moment
			
	/*
	* When a stage set (phase) is completed, if there is more sets avalible, 
	* the game will go to PrepState. In this case, PrepState needs to know 
	* The list of units used during this set, so that they may not be reused in the next set
	* 
	* StageState will handle keeping the wallet, challenges, and stageRecord intact.
	*/
	std::vector<int> usedUnits;
	for (int i = 0; i < loadout.filledSlots; i++) 
		usedUnits.push_back(loadout.slots[i].unitStats.id);

	nextStateEnterData = std::make_unique<StageSetPrepEnterData>(usedUnits, stageId, curStageSet);
	readyToEndState = true;
}

void StageState::on_enter(OnStateEnterData* enterData) {
	if (auto stageData = dynamic_cast<StageEnterData*>(enterData)) {
		std::ifstream stageFile(stageData->stageJsonPath);
		nlohmann::json stageJson = nlohmann::json::parse(stageFile);
		stageFile.close();

		loadout.create_loadout(stageData->slots);
		loadout.set_slot_positions(cam);
		stageManager.create_stage(stageJson);

		cam.reset();
		cam.change_lock(false);
	}
	else {
		std::cout << "enterData is not of type [StageEnterData]" << std::endl;
		return;
	}
}
void StageState::on_exit() {
	std::cout << "unloading stage" << std::endl;
	stageManager.stage = {};

	stageUI.paused = false;
	for (int i = 0; i < UI::StageUI::BTN_COUNT; i++)
		stageUI.buttonManager.buttons[i].sprite.setColor(sf::Color::White);
}