#pragma once
#include "EventTypes.h"

struct Event {
	EventType type = EventType::NONE;

	Event() = default;
	virtual ~Event() = default;
};
struct TimedEvent : Event {
	float timePassed = 0.0f;

	explicit TimedEvent(float time) : timePassed(time) {
		type |= EventType::TIMED_UPDATE;
	}

	static TimedEvent oneSecond;
};

struct IObserver {
public:
	IObserver() = default;
	virtual ~IObserver() = default;
	virtual bool notify(const Event* event) = 0; 
};
