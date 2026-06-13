#include "scheduling_algorithm.h"

#include "task.h"
#include "time_blocks.h"

#include <vector>
#include <chrono>

using namespace std::chrono_literals;

std::vector<FinalBlock> get_schedule(std::vector<TimeBlock> time_blocks, std::vector<Task> tasks) {
    //
    auto algo_start = std::chrono::system_clock::now();
    std::chrono::zoned_time local_algo_start(std::chrono::current_zone(), algo_start);
    algo_start = local_algo_start.get_sys_time();

    return {};
}