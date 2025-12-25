#pragma once

struct Event {
	//Will have eventType enum for ACHIEVMENTS
	int value = 0;
	void* ptr = nullptr;

	Event() = default;
	Event(int val, void* ptr) : value(val), ptr(ptr) {}
};
struct Observer {
public:
	Observer() = default;
	virtual ~Observer() = default;
	virtual bool notify(const Event& event) = 0;
};
