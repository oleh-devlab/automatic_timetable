#include "task.h"
#include "time_blocks.h"

FinalBlock::FinalBlock(const Task& src_task, TimeInterval interval, std::string algo_notes, bool is_task) : task_id(src_task.id), interval(interval), algo_notes(algo_notes), is_task(is_task) {}