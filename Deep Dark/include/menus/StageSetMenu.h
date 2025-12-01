#pragma once
#include "Menu.h"
#include "UnitData.h"

// Will be owned by the Armroy Menu as a big overlay/draw.
// IT will comprise of the background to show the enemys, the small sprites
// of said enemies, text asking if they want to move on, annd three buttons

// BTN #1: Accept. Starts the next Stage set
// BTN #2: Cancel. Closes StageSetMenu and goes back to armory
// BTN #3: Exit. Completely closes StageSet section and goes to stage Select screen.
const int STAGE_SET_BUTTONS = 3;

struct StageSetMenu : public Menu<STAGE_SET_BUTTONS> {
    std::bitset<UnitData::TOTAL_PLAYER_UNITS> usedUnits;
    int stageId = 0;
    int stageSet = 1;

    sf::Sprite backgroundSprite = sf::Sprite(defTex);

    sf::Text startStageSetText = sf::Text(baseFont);
    std::vector<sf::Texture> unitTextures = {};
    std::vector<sf::Sprite> enemyUnitSprites = {};
    std::array<sf::Sprite, 10> usedUnitsSlotSprites = make_unitSlotSprites();

    explicit StageSetMenu(Camera& cam);
    ~StageSetMenu() final = default;

    void reset_positions() final;
    void setup_menu(const std::vector<int>& ids, int stage, int set);

    inline Button& startStageBtn() { return buttonManager.buttons[0]; }
    inline Button& closeBtn() { return buttonManager.buttons[1]; }
    inline Button& exitStageBtn() { return buttonManager.buttons[2]; }

    static std::array<sf::Sprite, 10> make_unitSlotSprites() {
        const sf::Texture& tex = TextureManager::t_defaultUnitSlot;
        return {
            sf::Sprite(tex), sf::Sprite(tex),sf::Sprite(tex),sf::Sprite(tex),
            sf::Sprite(tex),sf::Sprite(tex),sf::Sprite(tex),sf::Sprite(tex),
            sf::Sprite(tex),sf::Sprite(tex)
        };
    }

};