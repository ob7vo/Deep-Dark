#pragma once
#include "TextureManager.h"

struct ArmorySlot {
    sf::Vector2f pos = { 0.f,0.f };
    sf::Vector2f bounds = { 0.f,0.f };
    sf::Sprite sprite = sf::Sprite(defTex);

    int id = -1; // -1 = invalid id
    int gear = 1; // 1 = base form
    int core = -1; // -1 = no core

    ArmorySlot() { clear(); }

    void set_unit(int ID, int g, int c = -1);
    void set_unit(std::pair<int, int> unit);
    void increase_gear() {
        gear = std::clamp(gear + 1, 1, 3);
    }

    void set_pos(sf::Vector2f _pos);
    inline void clear() { set_unit(-1, 1, -1); }

    std::pair<int, unsigned int> slotted_unit() { return { id, gear }; }
    bool hovered_over(sf::Vector2i mPos) const;
    inline bool empty() const { return id == -1; }

    static std::array<ArmorySlot, 10> default_armory_loadout();
};