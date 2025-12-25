#pragma once
#include "EnumFunctions.h"
#include <stdint.h>

enum class UnitAnimationState {
	WAITING = -4,
	PHASE_WINDDOWN = -3,
	PHASE_WINDUP = -2,
	SUMMON = -1,
	MOVE = 0,
	ATTACK = 1,
	IDLE = 2, // Any AnimationState below this make the Unit INVINCIBLE (title card)
	KNOCKBACK = 3,
	FALLING = 4,
	JUMPING = 5,
	PHASE_ACTIVE = 6,
	TRANSFORM = 7,
	DEATH
};
enum class DeathCause {
	NONE = 0,
	UNIT,
	SURGE,
	FALLING,
	TRAP,
	CANNON,
	BASE_WAS_DESTROYED
};
enum class UnitType : std::uint32_t { // comments depict the Types common builds
	NONE = 0,
	TYPELESS = 1 << 0, // any architype
	STEEL = 1 << 1, // High damage per hit and good health, low range, mid knockbacks (reds)
	NANO = 1 << 2, // Fast, high dps, high knockback count. Don't TRIGGER traps (can still be hurt by them)
	RUSTED = 1 << 3, // Very Slow and VERY bulky, will idle on gaps
	FLOATING = 1 << 4, // Agile, generally mid-ranged, ignores gaps 
	REANIMEN = 1 << 5, // Fast, low range, high dps, low health, can revive.
	ANCIENT = 1 << 6, // Ignore teleporters, usaully apply short-circuit, good bulk and damage.
	HOLOGRAM = 1 << 7, // Can phase through units, good at applying statuses
	VOIDED = 1 << 8, // Aku-sheilds and have unique voided ability.
	ALL = 1 << 9 // Every Unit has type ALL, easy way to give target to all types
};
enum class UnitSpawnType {
	NORMAL,
	DROPBOX,
	REVIVE,
	BOSS,
	SUMMON,
	TRANSFORMATION
};

constexpr UnitType operator|(UnitType a, UnitType b) {
	return static_cast<UnitType>(
		static_cast<std::uint32_t>(a) |
		static_cast<std::uint32_t>(b)
		);
}
constexpr UnitType operator&(UnitType a, UnitType b) {
	return static_cast<UnitType>(
		static_cast<std::uint32_t>(a) &
		static_cast<std::uint32_t>(b)
		);
}
constexpr UnitType operator~(UnitType a) {
	using U = std::underlying_type_t<UnitType>;
	return static_cast<UnitType>(~static_cast<U>(a));
}
constexpr UnitType& operator|=(UnitType& a, UnitType b) {
	a = a | b;
	return a;
}

constexpr DeathCause operator|(DeathCause a, DeathCause b) {
	return static_cast<DeathCause>(
		static_cast<std::uint32_t>(a) |
		static_cast<std::uint32_t>(b)
		);
}
constexpr DeathCause operator&(DeathCause a, DeathCause b) {
	return static_cast<DeathCause>(
		static_cast<std::uint32_t>(a) &
		static_cast<std::uint32_t>(b)
		);
}
constexpr DeathCause operator~(DeathCause a) {
	using U = std::underlying_type_t<DeathCause>;
	return static_cast<DeathCause>(~static_cast<U>(a));
}
constexpr DeathCause& operator|=(DeathCause& a, DeathCause b) {
	a = a | b;
	return a;
}