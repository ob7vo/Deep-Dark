#include "pch.h"
#include "Scheduler.h"

void Scheduler::schedule(float delaySeconds, Task task) {
    tasks.push_back({ delaySeconds, std::move(task) });
}

void Scheduler::update(float dt) {
    for (size_t i = tasks.size(); i--;) {
        auto& t = tasks[i];
        t.remaining -= dt;

        if (t.remaining <= 0.f) {
            t.task();
            tasks[i] = std::move(tasks.back());
            tasks.pop_back();
        }
    }
}