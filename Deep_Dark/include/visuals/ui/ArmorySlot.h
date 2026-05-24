#pragma once
#include "TextureManager.h"
#include <SFML/Graphics/Sprite.hpp>
#include "UnitConfigConstants.h"

class ArmoryMenu;
class Camera;

struct ArmorySlot {
    sf::Vector2f pos = { 0.f,0.f };
    sf::Vector2f bounds = { 0.f,0.f };
    sf::Sprite sprite = sf::Sprite(defaultTexture);

    int id = -1; // -1 = invalid id
    int core = -1; // -1 = no core

    bool usable = true;

    ArmorySlot() = default;

    void set_unit(int ID, bool usauble, int c = -1);
    void copy(ArmorySlot& moveSlot);

    void set_pos(sf::Vector2f _pos);
    inline void clear() { set_unit(-1, false, -1); }

    bool hovered_over(sf::Vector2i mPos) const;
    inline bool empty() const { return id == -1; }

    static std::array<ArmorySlot, UnitConfig::MAX_EQUIP_SLOTS> default_armory_loadout(Camera& cam);
};