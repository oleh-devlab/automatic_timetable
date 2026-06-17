#include "schedule_control.h"

#include <iostream>
#include <chrono>
#include <cstdio>

#include "../utils/time_utils.h"

namespace {

// Duration in minutes between two points.
long long duration_min(const TimeInterval& iv) {
    return std::chrono::duration_cast<std::chrono::minutes>(iv.end - iv.start).count();
}

} // anonymous namespace

const void printSchedule(std::vector<FinalBlock> schedule) {
    if (schedule.empty()) {
        std::cout << "  Schedule is empty.\n";
        return;
    }

    std::cout << "\n";
    std::cout << "============================================================\n";
    std::cout << "                       SCHEDULE\n";
    std::cout << "============================================================\n\n";

    for (size_t i = schedule.size(); i-- > 0; ) {
        const auto& block = schedule[i];
        auto dur = duration_min(block.interval);

        if (block.is_task) {
            std::cout << "  ▶ " << block.task_name;
            if (block.total_sessions > 1) {
                std::cout << "  [session " << block.session_index
                          << "/" << block.total_sessions << "]";
            }
            std::cout << "\n";
            std::cout << "    " << fmt_datetime(block.interval.start)
                      << " → " << fmt_datetime(block.interval.end)
                      << "  (" << dur << " min)\n";
        } else {
            std::cout << "  ○ " << (block.algo_notes.empty() ? "Break" : block.algo_notes)
                      << "\n";
            if (dur > 0) {
                std::cout << "    " << fmt_datetime(block.interval.start)
                          << " → " << fmt_datetime(block.interval.end)
                          << "  (" << dur << " min)\n";
            }
        }

        if (!block.algo_notes.empty() && block.is_task) {
            std::cout << "    !!! " << block.algo_notes << "\n";
        }

        std::cout << "  ----------------------------------------------------------\n";
    }
    std::cout << "\n";
}
