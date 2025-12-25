#pragma once
#include "Menu.h"
#include "UnitData.h"
#include "StageRestrictions.h"
#include <bitset>
#include <array>

// Will be owned by the Armroy Menu as a big overlay/draw.
// IT will comprise of the background to show the enemys, the small sprites
// of said enemies, text asking if they want to move on, annd three buttons

// BTN #1: Accept. Starts the next Stage set
// BTN #2: Cancel. Closes StageSetMenu and goes back to armory
// BTN #3: Exit. Completely closes StageSet section and goes to stage Select screen.
const int STAGE_SET_BUTTONS = 3;

struct StageSetMenu : public Menu<STAGE_SET_BUTTONS> {
    std::bitset<UnitData::TOTAL_PLAYER_UNITS> usedUnits;
    // First index is the specific Unit Id, the bitset is its gear (form)
    UnitRestrictions unitRestrictions;

    int stageId = 0;
    int stageSet = 0;
    
    sf::Sprite backgroundSprite = sf::Sprite(defTex);

    sf::Text startStageSetText = sf::Text(baseFont);
    std::vector<sf::Texture> enemyUnitTextures = {};
    std::vector<sf::Sprite> enemyUnitSprites = {};
    std::array<sf::Sprite, 10> usedUnitsSlotSprites = make_unitSlotSprites();

    explicit StageSetMenu(Camera& cam);
    ~StageSetMenu() final = default;

    void reset_positions() final;
    void draw() final;

    void setup_menu(int stage, int set = {}, const std::array<std::pair<int,int>, 10>& units = emptyUnits());
    void create_enemy_sprites(const nlohmann::json& stageSetJson);

    void reset_sprites();
    void full_reset();

    inline Button& startStageBtn() { return buttonManager.buttons[static_cast<int>(UI::StageSet::ButtonIndex::START_STAGE)]; }
    inline Button& closeBtn() { return buttonManager.buttons[static_cast<int>(UI::StageSet::ButtonIndex::CLOSE)]; }
    inline Button& exitStageBtn() { return buttonManager.buttons[static_cast<int>(UI::StageSet::ButtonIndex::EXIT_STAGE)]; }

    static std::array<sf::Sprite, 10> make_unitSlotSprites();
    /// <summary> Exists so I dont get the 'Invalid Gear Value' error </summary>
    static std::array<std::pair<int, int>, 10> emptyUnits(){
        std::array<std::pair<int,int>, 10> units;
        for (int i = 0; i < 10; i++) units[i] = { -1, 1 };
        return units;
    }
};