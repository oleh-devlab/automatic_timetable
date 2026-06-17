#include "task.h"
#include "time_blocks.h"

#include <utility>

FinalBlock::FinalBlock(const Task& src_task,
                       TimeInterval interval,
                       int session_index,
                       int total_sessions,
                       std::string algo_notes)
    : task_id(src_task.id)
    , task_name(src_task.name)
    , interval(interval)
    , is_task(true)
    , algo_notes(std::move(algo_notes))
    , session_index(session_index)
    , total_sessions(total_sessions)
{}

FinalBlock::FinalBlock(TimeInterval interval,
                       std::string algo_notes)
    : task_id(0)
    , task_name()
    , interval(interval)
    , is_task(false)
    , algo_notes(std::move(algo_notes))
    , session_index(0)
    , total_sessions(0)
{}