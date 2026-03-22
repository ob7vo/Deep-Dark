#pragma once
#include "GameState.h"
#include "StageManager.h"

class ArmoryMenu;
struct ArmorySlot;
struct StageEnterData;

// State for when a stage is being played
// When exiting this state, the only data changed is the stage itself (set to nullptr), and it being unpaused.
// It will keep the stageManager intact so that challenges and the recorder will persist between phases
class StageState : public GameState {
private:
	StageUI stageUI;
	Loadout loadout;
	StageManager stageManager;
public:
	int curStageID = 0;
	int curStagePhase = 0;
	int stagePhasesCount = 1;
	bool inPracticeMode = false;

	explicit StageState(Camera& cam);
	~StageState() override = default;

	void update(float deltaTime) override;
	void render() override;
	void on_enter(OnStateEnterData* enterData) override;
	void on_exit() override;

	void update_ui(float deltaTime) override;
	void transition_from_results_screen(float deltaTime);
	void transition_to_results_screen(float deltaTime);
	void test_stage_ui_transition(float deltaTime);
	void check_transition(float deltaTime) override;

	void handle_events(sf::Event event) override;

	void handle_stage_victory();
	/// <summary>
	/// Needed to fully exit the stage. Only called via result screen quit button, or
	/// from the quit button in the armory when preparing for the next stage set
	/// </summary>
	void quit_stage();
	void end_current_stage_phase(bool playerWon);
	// Enters armory menu where the player will equip new units for the next phase (set)
	void enter_armory_from_results_screen();

	void start_stage_phase(const StageEnterData* stageEntry);
	void restart_stage_phase();
};

struct StageEnterData : public OnStateEnterData {
	int stageID;
	int stagePhase;
	const std::array<ArmorySlot, UnitConfig::MAX_EQUIP_SLOTS>& slots;
	bool inPracticeMode = false;

	StageEnterData(int stageID, int phase, const std::array<ArmorySlot, UnitConfig::MAX_EQUIP_SLOTS>& slots, bool practice);

	~StageEnterData() override = default;
};