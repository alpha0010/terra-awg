#include "QueuedTasks.h"

#include "Random.h"
#include <algorithm>
#include <cassert>

void QueuedTasks::addTask(RenderTask &&task)
{
    assert(!isComplete);
    tasks.emplace_back(task);
}

void QueuedTasks::runTasks(Random &rnd, World &world)
{
    assert(!isComplete);
    isComplete = true;
    std::shuffle(tasks.begin(), tasks.end(), rnd.getPRNG());
    for (const auto &task : tasks) {
        task(rnd, world);
    }
    tasks.clear();
}
