#include <vector>

#include "structs.h"
#include "scheduling_algorithm.h"

class Schedule {
private:
    std::vector<TimeBlock> time_blocks;
    std::vector<Task> tasks;
public:
    Schedule(std::vector<TimeBlock> time_blocks, std::vector<Task> tasks);

    std::vector<FinalTask> get_schedule();
};