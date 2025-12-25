#pragma once
#include "Menu.h"
#include "Slider.h"
#include "ArmorySlot.h"
#include "StageSetMenu.h"
#include <bitset>

const int ARMORY_SLOTS = 10;

class ArmoryMenu : public Menu<UI::ArmoryMenu::BTN_COUNT> {
public:
    bool inStageMode = false;
    StageSetMenu stageSetMenu;
    
    sf::Text armoryText = sf::Text(baseFont);
    Slider unitSlider = {};
    
    std::array<ArmorySlot, ARMORY_SLOTS> slots;
    int filledUnitSlots = 0;

    /// <summary> first = unit ID. second = unit form/gear </summary>
    std::pair<int, int> curHeldUnit = { -1,1 };
    bool heldUnitWasSlotted = false;
    // The forms of the Units shown in the selector
    std::array<int, UnitData::TOTAL_PLAYER_UNITS> displayedGears;
    
    explicit ArmoryMenu(Camera& cam);
    ~ArmoryMenu() final = default;

    void draw() final;
    void check_mouse_hover() final;
    bool on_mouse_press(bool isM1) final;
    bool on_mouse_release(bool isM1) final;
    void reset_positions() final;
    void update(float deltaTime) final;

    void drag_unit_into_slot();
    void start_dragging_unit(int id);
    void shift_empty_slots();
    void remove_unusable_units();
    /// <summary>
    /// Called when a Units gear is changed via Workshop Menu, it can be a 
    /// either a player or enemy unit, in the latter case it return
    /// </summary>
    void change_displayed_gear(int id, int gear);

    inline bool dragging_unit() const { return curHeldUnit.first != -1; }
    inline void release_held_unit() { curHeldUnit = { -1, 1 }; }

    inline bool unit_is_equipped(int id) const {
        for (int i = 0; i < 10; i++)
            if (slots[i].id == id) return true;
        return false;
    } 
    inline bool unit_is_unusable(int id, int gear) const 
    { return stageSetMenu.usedUnits[id] || stageSetMenu.unitRestrictions.is_allowed(id, gear); }
    inline bool unit_is_unusable(std::pair<int, int> unit) const {
        return unit_is_unusable(unit.first, unit.second);
    }

    inline Slider& slider() { return unitSlider; }
    inline Button& returnBtn() { return buttonManager.buttons[static_cast<int>(UI::ArmoryMenu::ButtonIndex::RETURN)]; }
    inline Button& openStageSetBtn() { return buttonManager.buttons[UI::ArmoryMenu::BTN_COUNT - 1]; }
    inline Button& unitSelectionBtn(int i) { return buttonManager.buttons[i]; }
};
// The Expression '_Param_(1)<5' is not true at this call