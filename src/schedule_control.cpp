#include "schedule_control.h"

#include <iostream>
#include <chrono>
#include <cstdio>

#include "time_utils.h"

namespace {

// Duration in minutes between two points.
long long duration_min(const TimeInterval& iv) {
    return std::chrono::duration_cast<std::chrono::minutes>(iv.end - iv.start).count();
}

} // anonymous namespace

const void printSchedule(std::vector<FinalBlock> schedule) {
    if (schedule.empty()) {
        std::cout << "  Розклад порожній.\n";
        return;
    }

    std::cout << "\n";
    std::cout << "============================================================\n";
    std::cout << "                       РОЗКЛАД\n";
    std::cout << "============================================================\n\n";

    for (size_t i = schedule.size(); i-- > 0; ) {
        const auto& block = schedule[i];
        auto dur = duration_min(block.interval);

        if (block.is_task) {
            std::cout << "  ▶ " << block.task_name;
            if (block.total_sessions > 1) {
                std::cout << "  [сесія " << block.session_index
                          << "/" << block.total_sessions << "]";
            }
            std::cout << "\n";
            std::cout << "    " << fmt_datetime(block.interval.start)
                      << " → " << fmt_datetime(block.interval.end)
                      << "  (" << dur << " хв)\n";
        } else {
            std::cout << "  ○ " << (block.algo_notes.empty() ? "Перерва" : block.algo_notes)
                      << "\n";
            if (dur > 0) {
                std::cout << "    " << fmt_datetime(block.interval.start)
                          << " → " << fmt_datetime(block.interval.end)
                          << "  (" << dur << " хв)\n";
            }
        }

        if (!block.algo_notes.empty() && block.is_task) {
            std::cout << "    ⚠ " << block.algo_notes << "\n";
        }

        std::cout << "  ----------------------------------------------------------\n";
    }
    std::cout << "\n";
}
