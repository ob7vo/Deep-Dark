#pragma once
#include <string_view>

enum class AnimationEvent : std::uint32_t {
	EMPTY_EVENT = 0,
	FIRST_FRAME = 1 << 0,
	FINAL_FRAME = 1 << 1,
	ATTACK = 1 << 2,
	TRIGGER = 1 << 3,
	TRIGGER_2 = 1 << 4
};
constexpr AnimationEvent operator|(AnimationEvent a, AnimationEvent b) {
    return static_cast<AnimationEvent>(
        static_cast<std::uint32_t>(a) |
        static_cast<std::uint32_t>(b)
        );
}
constexpr AnimationEvent operator&(AnimationEvent a, AnimationEvent b) {
    return static_cast<AnimationEvent>(
        static_cast<std::uint32_t>(a) &
        static_cast<std::uint32_t>(b)
        );
}
constexpr AnimationEvent& operator|=(AnimationEvent& a, AnimationEvent b) {
    a = a | b;
    return a;
}
constexpr bool any(AnimationEvent e) {
    return static_cast<std::uint32_t>(e) != 0;
}

using AnimationEventsList = std::vector<std::pair<int, AnimationEvent>>;

namespace AnimationEvents {
    inline AnimationEvent fromString(std::string_view str) {
        if (str == "first_frame") return AnimationEvent::FIRST_FRAME;
        if (str == "final_frame") return AnimationEvent::FINAL_FRAME;
        if (str == "attack") return AnimationEvent::ATTACK;
        if (str == "trigger") return AnimationEvent::TRIGGER;
        if (str == "trigger_2") return AnimationEvent::TRIGGER_2;

        return AnimationEvent::EMPTY_EVENT;
    }

    inline int fromStringMask(std::string_view str) {
        return static_cast<int>(fromString(str));
    }
}