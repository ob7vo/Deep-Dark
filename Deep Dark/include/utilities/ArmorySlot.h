#pragma once
#include "SFML/Graphics.hpp"
#include "TextureManager.h"

struct ArmorySlot {
    sf::Vector2f pos = { 0.f,0.f };
    sf::Vector2f bounds = { 0.f,0.f };
    sf::Sprite sprite = sf::Sprite(defTex);
    sf::Texture texture = {};

    int id = -1;
    int gear = 0;
    int core = -1;

    ArmorySlot() { clear(); }

    void set_unit(int ID, int g, int c = -1);
    inline void set_unit(std::pair<int, int> unit) {
        set_unit(unit.first, unit.second);
    }
    inline void increase_gear() {
        gear++;
    }

    inline void set_pos(sf::Vector2f _pos) {
        pos = _pos;
        bounds = bounds = sprite.getGlobalBounds().size * 1.05f;
        sprite.setPosition(_pos);
    }
    inline void clear() { set_unit(-1, 0, -1); }

    inline std::pair<int, unsigned int> slotted_unit() { return { id, gear }; }
    inline bool hovered_over(sf::Vector2i mPos) const {
        return static_cast<float>(mPos.x) >= pos.x - bounds.x * 0.5f
            && static_cast<float>(mPos.x) <= pos.x + bounds.x * 0.5f
            && static_cast<float>(mPos.y) >= pos.y - bounds.y * 0.5f
            && static_cast<float>(mPos.y) <= pos.y + bounds.y * 0.5f;
    }
    inline bool empty() const { return id == -1; }

    static std::array<ArmorySlot, 10> default_armory_loadout() {
        std::array<ArmorySlot, 10> slots;

        slots[0].set_unit(0, 3, 0);
        slots[1].set_unit(1, 2);
        for (int i = 2; i < 10; i++) 
            slots[i].set_unit(-1, -1);

        return slots;
    }
};