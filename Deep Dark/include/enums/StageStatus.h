#pragma once
#include "EnumFunctions.h"

enum class StageStatus : std::int32_t {
	ONGOING = 1 << 0,
	FINISHED = 1 << 1,
	ENEMY_VICTORY = 1 << 2,
	PLAYER_VICTORY = 1 << 3,
	WAITING_FOR_EXIT = 1 << 4
};

constexpr StageStatus operator|(StageStatus a, StageStatus b) {
	return static_cast<StageStatus>(
		static_cast<std::int32_t>(a) |
		static_cast<std::int32_t>(b)
		);
}
constexpr StageStatus operator&(StageStatus a, StageStatus b) {
	return static_cast<StageStatus>(
		static_cast<std::int32_t>(a) &
		static_cast<std::int32_t>(b)
		);
}
constexpr StageStatus operator~(StageStatus a) {
	using U = std::underlying_type_t<StageStatus>;
	return static_cast<StageStatus>(~static_cast<U>(a));
}

constexpr StageStatus& operator|=(StageStatus& a, StageStatus b) {
	a = a | b;
	return a;
}