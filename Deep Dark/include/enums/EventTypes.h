#pragma once
#include "EnumFunctions.h"

enum class EventType : std::uint32_t {
	NONE,
	TUTORIAL_PROMPT_1,
	TIMED_UPDATE,
	UNIT_ABILITY,
	UNIT_OVERCLOCK_READY
};

constexpr EventType operator|(EventType a, EventType b) {
	return static_cast<EventType>(
		static_cast<std::uint32_t>(a) |
		static_cast<std::uint32_t>(b)
		);
}
constexpr EventType operator&(EventType a, EventType b) {
	return static_cast<EventType>(
		static_cast<std::uint32_t>(a) &
		static_cast<std::uint32_t>(b)
		);
}
constexpr EventType operator~(EventType a) {
	using U = std::underlying_type_t<EventType>;
	return static_cast<EventType>(~static_cast<U>(a));
}

constexpr EventType& operator|=(EventType& a, EventType b) {
	a = a | b;
	return a;
}