#pragma once
#include "Menu.h"
#include "Slider.h"
#include "ArmorySlot.h"
#include "UnitData.h"

const int ARMORY_BTNS = UnitData::TOTAL_PLAYER_UNITS + 1;
const int ARMORY_SLOTS = 10;

struct ArmoryMenu : public Menu<ARMORY_BTNS> {
    sf::Text armoryText = sf::Text(baseFont);
    Slider unitSlider = {};
    
    std::array<ArmorySlot, ARMORY_SLOTS> slots;
    int filledUnitSlots = 0;

    /// <summary> first = unit ID. second = unit form/gear </summary>
    std::pair<int, int> curHeldUnit = { -1,-1 };
    bool heldUnitWasSlotted = false;
    std::array<int, UnitData::TOTAL_PLAYER_UNITS> unitSelectionForms;
    
    explicit ArmoryMenu(Camera& cam);
    ~ArmoryMenu() = default;
    void set_up_buttons();

    void draw() override;
    void check_mouse_hover() override;
    bool on_mouse_press(bool isM1) override;
    bool on_mouse_release(bool isM1) override;
    void reset_positions() override;
    void update(float deltaTime) override;

    void drag_unit_into_slot();
    void drag_unit(int id);
    void shift_empty_slots();

    inline bool dragging_unit() const { return curHeldUnit.first != -1; }
    inline bool unit_is_slotted(int id) const {
        for (int i = 0; i < 10; i++)
            if (slots[i].id == id) return true;
        return false;
    } 
    inline void release_hold() { curHeldUnit = { -1,-1 }; }

    inline Slider& slider() { return unitSlider; }
    inline Button& returnBtn() { return buttonManager.buttons[UnitData::TOTAL_PLAYER_UNITS]; }
    inline Button& unitSelectionBtn(int i) { return buttonManager.buttons[i]; }
};