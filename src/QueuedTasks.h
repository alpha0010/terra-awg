#ifndef QUEUEDTASKS_H
#define QUEUEDTASKS_H

#include <functional>
#include <vector>

class World;
class Random;

typedef std::function<void(Random &, World &)> RenderTask;

class QueuedTasks
{
private:
    bool isComplete = false;
    std::vector<RenderTask> tasks;

public:
    void addTask(RenderTask &&task);
    void runTasks(Random &rnd, World &world);
};

#endif // QUEUEDTASKS_H
