#pragma once
#include <SFML/System/Vector2.hpp>

namespace sf {
    template <typename T>
    constexpr sf::Vector2<T> operator*(sf::Vector2<T> a, sf::Vector2<T> b) {
        return sf::Vector2<T>( a.x * b.x, a.y * b.y );
    }
}