#pragma once
#include <type_traits>

template <typename E>
constexpr bool has(E mask, E bit) noexcept {
    static_assert(std::is_enum_v<E>, "has() requires enum type");
    using U = std::underlying_type_t<E>;
    return (static_cast<U>(mask) & static_cast<U>(bit)) != 0;
}

template <typename E>
constexpr bool has(E e) noexcept {
    static_assert(std::is_enum_v<E>, "has() requires enum type");
    return static_cast<std::underlying_type_t<E>>(e) != 0;
}

template <typename E>
constexpr void remove(E& mask, E bit) noexcept {
    static_assert(std::is_enum_v<E>, "has() requires enum type");
    using U = std::underlying_type_t<E>;
    mask = static_cast<E>(static_cast<U>(mask) & ~static_cast<U>(bit));
}
