#ifndef TASK_H
#define TASK_H

#include "time_blocks.h"

#include <cstdint>
#include <string>
#include <chrono>
#include <optional>

// Represents a single task provided by the user.
struct Task {
    uint64_t id;
    std::string name;
    std::string description;

    std::optional<PointInTime> deadline;
    int priority = 0;

    // Total estimated working time required to complete the task.
    std::chrono::minutes total_duration{0};

    std::chrono::minutes work_session_duration{25}; // Preferred duration of a single uninterrupted work session (sub-task).
    std::chrono::minutes break_duration{5};
    std::optional<std::chrono::minutes> min_session_duration;
    bool allow_shrink = false;
};

// A single block in the resulting schedule produced by the algorithm.
struct FinalBlock {
    uint64_t task_id; // id of the original Task (0 for autonomous breaks)
    std::string task_name; // copied from Task::name for convenience
    TimeInterval interval;
    bool is_task; // true = work session, false = break / blocked time

    std::string algo_notes; // Notes left by the algorithm

    int session_index = 0; // For breaks this is 0.
    int total_sessions = 0; // Total number of sessions planned for the parent Task.

    // Work-session constructor
    FinalBlock(const Task& src_task,
               TimeInterval interval,
               int session_index,
               int total_sessions,
               std::string algo_notes = "");

    // Break / non-task block constructor
    FinalBlock(TimeInterval interval,
               std::string algo_notes = "");
};

#endif // TASK_H