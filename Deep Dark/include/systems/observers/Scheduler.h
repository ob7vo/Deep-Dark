#pragma once
#include <functional>
#include <vector>

class Scheduler {
public:
    using Task = std::function<void()>;

    Scheduler() = default;

    void schedule(float delaySeconds, Task task);
    void update(float dt);
private:
    struct ScheduledTask {
        float remaining;
        Task task;
    };

    std::vector<ScheduledTask> tasks;
};