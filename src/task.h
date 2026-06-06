#ifndef TASK_H
#define TASK_H

#include "time_blocks.h"

struct Task {
    uint64_t id;
    std::string name;
    std::string description;
    // Settings that will help the algorithm fit this task into the schedule
};

struct FinalBlock {
    uint64_t task_id; // id of original task
    std::string algo_notes; // Notes on the algorithm. For example, ...
    // ... it did not increase the runtime because it was unable to generate a schedule with the increase.
    TimeInterval interval;
    bool is_task;
    
    FinalBlock(const Task& src_task, TimeInterval interval, std::string algo_notes, bool is_task);
};

#endif // TASK_H