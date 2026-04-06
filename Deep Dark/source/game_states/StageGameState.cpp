#include "pch.h"
#include "StageGameState.h"
#include "StageSaveData.h"
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

	stageUI.resultsScreen.nextStageSetBtn().onClick = [this](bool m1) {
		if (m1) {
			stageUI.resultsScreen.clickable = false;
			start_transition(TransitionID::ResultsScreenToArmory);
		}
		};
	stageUI.resultsScreen.quitStageBtn().onClick = [this](bool m1) {
		if (m1) {
			stageUI.resultsScreen.clickable = false;
			start_transition(TransitionID::ResultsScreenToStageSelect);
		}
		};
	stageUI.resultsScreen.restartStageSetBtn().onClick = [this](bool isM1) {
		if (isM1) restart_stage_phase();
		};
}
StageEnterData::StageEnterData(int stageID, int phase, const std::array<ArmorySlot, UnitConfig::MAX_EQUIP_SLOTS>& slots, bool practice) :
	OnStateEnterData(GameState::Type::STAGE), 
	stageID(stageID), 
	stagePhase(phase), 
	slots(slots),
	inPracticeMode(practice)
	{}

void StageState::update_ui(float deltaTime) {
	for (int i = 0; i < loadout.filledSlots; i++)
		loadout.slots[i].cooldown -= deltaTime;

	stageUI.partsCountText.setString(std::format("${}", stageManager.wallet.parts));
	stageUI.bagUpgradeCostText.setString(std::format("#{}/{}", stageManager.wallet.parts, stageManager.wallet.partsCap));
}
void StageState::update(float deltaTime) {
	stageUI.check_mouse_hover();
	if (stageUI.paused) return;

	stageManager.tick(deltaTime);
	update_ui(deltaTime);

	check_transition(deltaTime);
}

void StageState::render() {
	stageManager.draw(cam.getWindow());
	loadout.draw_slots(cam, stageManager.wallet.parts);

	stageUI.draw();
}

void StageState::handle_events(sf::Event event) {
	// Won't run if the stage is finished
	stageManager.handle_events(event);

	if (auto click = event.getIf<sf::Event::MouseButtonPressed>())
		clicked = stageUI.on_mouse_press(is_M1(click));
	if (auto key = event.getIf<sf::Event::KeyPressed>()) {
		if (key->code == Key::T) {
			start_transition(TransitionID::TestStageUISliding);
		}
	}
}

void StageState::check_transition(float deltaTime) {
	switch (currentTransition) {
	case TransitionID::StageUIToResultsScreen: 
		transition_to_results_screen(deltaTime);
		break;
	case TransitionID::ResultsScreenToArmory:
	case TransitionID::ResultsScreenToStageSelect:
		transition_from_results_screen(deltaTime);
		break;
	case TransitionID::TestStageUISliding:
		test_stage_ui_transition(deltaTime);
		break;
	}
}
void StageState::transition_to_results_screen(float deltaTime) {
	// Get T
	float t = updateTransitionTime(deltaTime);

	// Move the StageUI offscreen with NO ease
	stageUI.slide(t);
	loadout.slide_ui(t);

	// Will add animation of the results screen opening when I can actually draw and animation
	// For now, just pop the results onscreen after the stageUI is done sliding offscreen
	if (t >= 1.f) {
		stageUI.resultsScreen.activate();

		// At this points, we do nothing but wait for the pLAyer to click a button to move on
		currentTransition = TransitionID::None;
	}
}
void StageState::transition_from_results_screen(float deltaTime) {
	/*
	// Get T
	transitionTimeElapsed += deltaTime;
	float t = transitionTimeElapsed / transitionTimer;

	// Again, since I cant animate, just pop the UI on screen. This time do it instantly
	*/
	float t = 1;

	stageUI.resultsScreen.deactivate();

	if (t >= 1.f) {
		// If the player pressed the "Quit Stage" button
		if (currentTransition == TransitionID::ResultsScreenToStageSelect)
			quit_stage();
		// If the plater pressed the
		else if (currentTransition == TransitionID::ResultsScreenToArmory)
			enter_armory_from_results_screen();

		currentTransition = TransitionID::None;
	}
}
void StageState::test_stage_ui_transition(float deltaTime) {
	// Get T
	float t = updateTransitionTime(deltaTime);

	// Move the StageUI offscreen with NO ease
	stageUI.slide(t);
	loadout.slide_ui(t);

	if (t >= 1.f) {
		stageUI.reset_positions();
		loadout.slide_ui(0.f); // sets its position to starting
	}
}

void StageState::quit_stage() {
	MenuType cur = MenuType::STAGE_SELECT;
	MenuType prev = MenuType::HOME_BASE;

	curStagePhase = 0;
	stagePhasesCount = 1;

	stageManager.challenges.clear();
	stageManager.stageRecorder = {};

	nextStateEnterData = std::make_unique<PrepEnterData>(cur, prev);
	readyToEndState = true;
}
void StageState::handle_stage_victory() {
	int currentWinsOnStage = StageSaveData::AddStageClear(curStageID);

	// First Clear
	if (currentWinsOnStage == 1) {
		StageSaveData::UnlockStages(curStageID);
	}

	StageSaveData::SetClearTime(curStageID, stageManager.stageRecorder.timeSinceStart);
	for (int i = 0; i < StageConfig::CHALLENGES_PER_STAGE; i++)
		if (stageManager.challenges[i].cleared) 
			StageSaveData::ClearChallenge(curStageID, i);
}
void StageState::end_current_stage_phase(bool playerWon) {
	// Deactivate the UI's usability and move it offscreen
	stageUI.set_pause_state(false);
	stageUI.clickable = false;
	start_transition(TransitionID::StageUIToResultsScreen);

	if (inPracticeMode) {
		stageUI.resultsScreen.setup_results_screen(playerWon, false, inPracticeMode);
		std::cout << curStagePhase << std::endl;
		return; 
	}
	else if (playerWon) curStagePhase++;

	bool completedAllPhases = curStagePhase >= stagePhasesCount;
	StageSaveData::SetHighestPhaseCleared(curStageID, curStagePhase);

	// Handle results
	std::cout << Printing::vec2(stageUI.resultsScreen.resultsMenu.getPosition()) << std::endl;
	stageUI.resultsScreen.setup_results_screen(playerWon, completedAllPhases, inPracticeMode);

	if (completedAllPhases) {
		handle_stage_victory(); // Saves new data
		return;
	}
}
void StageState::enter_armory_from_results_screen() {
	/*
	* When a stage phase is completed, if there is more sets available,
	* the game will go to PrepState. In this case, PrepState needs to know
	* The list of units used during this set, so that they may not be reused in the next set
	*
	* StageState will handle keeping the wallet, challenges, and stageRecord intact.
	*/
	nextStateEnterData = std::make_unique<EntryOnStagePhaseClear>(loadout, curStageID, curStagePhase);
	readyToEndState = true;
}

void StageState::start_stage_phase(const StageEnterData* stageEntry) {
	curStageID = stageEntry->stageID;
	curStagePhase = stageEntry->stagePhase;
	nlohmann::json stageJson = StageConfig::getStageJson(curStageID);

	loadout.create_loadout(stageEntry->slots);
	loadout.set_slot_positions(cam);

	stageManager.create_stage(stageJson, stageEntry->stagePhase, stageEntry->inPracticeMode);
	stageManager.stage->onStageCompletion = [this](bool victorious) {
		end_current_stage_phase(victorious);
		};

	stageUI.on_enter();

	inPracticeMode = stageEntry->inPracticeMode;

	cam.reset();
	cam.change_lock(false);
}
void StageState::restart_stage_phase() {
	std::cout << "restarting stage" << std::endl;

	nlohmann::json stageJson = StageConfig::getStageJson(curStageID);
	stageManager.create_stage(stageJson, curStagePhase, inPracticeMode);
	stageManager.stage->onStageCompletion = [this](bool victorious) {
		end_current_stage_phase(victorious);
		};

	loadout.set_slot_positions(cam);
	stageUI.on_enter();

	cam.reset();
	cam.change_lock(false);
}

void StageState::on_enter(OnStateEnterData* enterData) {
	if (auto stageData = dynamic_cast<StageEnterData*>(enterData)) {
		start_stage_phase(stageData);
	}
	else {
		std::cout << "enterData is not of type [StageEnterData]" << std::endl;
		return;
	}
}
void StageState::on_exit() {
	std::cout << "unloading stage" << std::endl;
	stageManager.stage = nullptr;

	stageUI.paused = false;
	for (int i = 0; i < UI::StageUI::BTN_COUNT; i++)
		stageUI.buttonManager.buttons[i].sprite.setColor(sf::Color::White);
}