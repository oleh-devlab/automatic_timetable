#ifndef SCHEDULE_CONTROL_H
#define SCHEDULE_CONTROL_H

#include <vector>

#include "task.h"
#include "time_blocks.h"
#include "scheduling_algorithm.h"

class Schedule {
private:
    std::vector<TimeBlock> time_blocks;
    std::vector<Task> tasks;
public:
    Schedule(std::vector<TimeBlock> time_blocks, std::vector<Task> tasks);

    std::vector<FinalTask> get_schedule();
};

const void printSchedule(std::vector<FinalTask> schedule);
#endif // SCHEDULE_CONTROL_H