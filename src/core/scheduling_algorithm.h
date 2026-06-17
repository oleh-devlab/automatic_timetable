#ifndef SCHEDULING_ALGORITHM_H
#define SCHEDULING_ALGORITHM_H

#include "task.h"
#include "time_blocks.h"
#include <vector>

std::vector<FinalBlock> get_schedule(std::vector<TimeBlock> time_blocks, std::vector<Task> tasks);

#endif // SCHEDULING_ALGORITHM_H