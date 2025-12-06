#include "pch.h"
#include "StageGameState.h"
#include "PreparationState.h"
#include "ArmoryMenu.h"

StageState::StageState(Camera& cam) : stageUI(cam), loadout(cam),
	GameState(cam), stageManager(cam, stageUI, loadout){
	stageUI.reset_positions();

	stageUI.stageManager = &stageManager;
	stageUI.pauseMenu.closeGameBtn().onClick = [this](bool m1) { if (m1) quit_stage(); };
}
StageEnterData::StageEnterData(const std::string& path, int set, const ArmoryMenu& armory) :
	OnStateEnterData(GameState::Type::STAGE), stageJsonPath(path), stageSet(set), slots(armory.slots) {}

void StageState::update_ui(float deltaTime) {
	for (int i = 0; i < loadout.filledSlots; i++)
		loadout.slots[i].cooldown -= deltaTime;

	stageUI.partsCountText.setString(std::format("#{}/{}", stageManager.wallet.parts, stageManager.wallet.partsCap));
}
void StageState::update(float deltaTime) {
	stageUI.check_mouse_hover();
	if (stageUI.paused) return;

	stageManager.update_game_ticks(deltaTime);
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
void StageState::end_stage_set() {
	if (++curStageSet >= stageSetCount) quit_stage();

	/*
	* When a stage set (phase) is completed, if there is more sets avalible, 
	* the game will go to PrepState. In this case, PrepState needs to know 
	* The list of units used during this set, so that they may not be reused in the next set
	* 
	* StageState will handle keeping the wallet, challenges, and stageRecord intact.
	*/
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

#pragma region Loadout Functions
void Loadout::set_slot_positions(const Camera& cam) {
	sf::Vector2f pos = cam.norm_to_pixels(FIRST_SLOT_POS);
	sf::Vector2f inc = cam.norm_to_pixels(SLOT_INCREMENT);
	float startX = pos.x;

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 5; j++) {
			int ind = i * 5 + j;
			LoadoutSlot& slot = slots[ind];
			slot.slotSprite.setPosition(pos);
			slot.slotSprite.setScale({ 2.5f,2.5f });

			pos.x += inc.x;
		}
		pos.x = startX;
		pos.y += inc.y;
	}
}
void Loadout::create_loadout(std::array<ArmorySlot, 10> armorySlots) {
	for (int i = 0; i < 10; i++) {
		ArmorySlot& slot = armorySlots[i];
		if (slot.id == -1) {
			filledSlots = i;
			break;
		}

		set_slot(slot.id, slot.gear, slot.core, i);
	}
	for (int j = filledSlots; j < 10; j++)
		slots[j] = LoadoutSlot(TextureManager::t_defaultUnitSlot);
}
void Loadout::set_slot(int id, int gear, int coreInd, int slot) {
	const nlohmann::json unitJson = UnitData::createUnitJson(id, gear);

	slots[slot] = LoadoutSlot(unitJson, TextureManager::getUnitSlot(id, gear), coreInd);
}
void Loadout::draw_slots(Camera& cam, int currentParts) {
	for (int i = 0; i < 10; i++)
		slots[i].draw(cam, currentParts);
}

LoadoutSlot::LoadoutSlot(const nlohmann::json& file, const sf::Texture& tex, int core)
	: empty(false), slotSprite(tex), unitStats(UnitStats::player(file, core))
{
	Animation::setup_unit_animation_map(file, aniMap);
	spawnTimer = unitStats.rechargeTime;
}
void LoadoutSlot::draw(Camera& cam, int curParts) {
	cam.queue_ui_draw(&slotSprite);

	float fill = 1.f;
	if ((cooldown <= 0 && can_afford_unit(curParts)) || empty) {
		cooldown = 0.f;
		return;
	}
	else if (cooldown > 0)
		fill = (spawnTimer - cooldown) / spawnTimer;

	cam.draw_overlay(slotSprite, UI::Colors::BLACK_TRANSPARENT, fill);
}
#pragma endregion