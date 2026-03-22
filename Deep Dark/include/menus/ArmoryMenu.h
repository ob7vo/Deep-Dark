#pragma once
#include "Menu.h"
#include "Slider.h"
#include "ArmorySlot.h"
#include "StagePreviewMenu.h"
#include "UnitConfigConstants.h"
#include <bitset>
#include <iostream>

class ArmoryMenu : public Menu<UI::ArmoryMenu::BTN_COUNT> {
public:

    enum class Mode {
        Normal,
        StagePreparation,
        ViewingStagePreview
    };

    Mode mode = Mode::Normal;
    StagePreviewMenu stagePreviewMenu;
    
    sf::Text armoryText = sf::Text(baseFont);
    Slider inventorySlider = {};
    
    std::array<ArmorySlot, UnitConfig::MAX_EQUIP_SLOTS> equipSlots;
    int filledUnitSlots = 0;

    /// <summary> first = unit ID. second = unit form/gear </summary>
    std::pair<int, int> curHeldUnit = { -1,1 };
    bool heldUnitWasSlotted = false;
    // The forms of the Units shown in the selector. "0" means its not unlocked/shown
    std::array<int, UnitConfig::TOTAL_PLAYER_UNITS> displayedGears;
    
    explicit ArmoryMenu(Camera& cam);
    ~ArmoryMenu() final = default;

    void draw() final;

    void check_mouse_hover() final;
    bool on_mouse_press(bool isM1) final;
    bool on_mouse_release(bool isM1) final;
    bool check_mouse_press_for_equip_slots(bool isM1, sf::Vector2i mouseScreenPos);

    void on_enter() final;
    void reset_positions() final;
    void update(float deltaTime) final;

    void drag_unit_into_slot();
    void start_dragging_unit(int id);
    void shift_empty_slots();
    /// <summary>
    /// Called when a Units gear is changed via Workshop Menu, it can be a 
    /// either a player or enemy unit, in the latter case it return
    /// </summary>
    void update_display_of_unit(int id, int gear);

    inline bool dragging_unit() const { return curHeldUnit.first != -1; }
    inline void release_held_unit() { curHeldUnit = { -1, 1 }; }

    inline bool unitIsOwned(int id) const { return displayedGears[id] != 0; }
    inline bool unit_is_equipped(int id) const {
        for (int i = 0; i < 10; i++)
            if (equipSlots[i].id == id) return true;
        return false;
    } 
    inline bool unit_is_unusable(int id, int gear) const 
    {
        if (stagePreviewMenu.usedUnits[id]) {
            std::cout << "used already" << std::endl;
            return true;
        }
        else if (!stagePreviewMenu.unitRestrictions.is_allowed(id, gear)) {
            std::cout << "restricted" << std::endl;
            return true;
        }
        return false;
    }
    inline bool unit_is_unusable(std::pair<int, int> unit) const {
        return unit_is_unusable(unit.first, unit.second);
    }

    inline Button& returnBtn() { return buttonManager.buttons[static_cast<int>(UI::ArmoryMenu::ButtonIndex::RETURN)]; }
    inline Button& openStageSetBtn() { return buttonManager.buttons[UI::ArmoryMenu::BTN_COUNT - 1]; }
    inline Button& inventorySlotBtn(int i) { return buttonManager.buttons[i]; }
};
// The Expression '_Param_(1)<5' is not true at this call