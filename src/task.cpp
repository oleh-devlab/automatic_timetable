#include "task.h"
#include "time_blocks.h"

FinalTask::FinalTask(const Task& src_task, TimeInterval interval, std::string algo_notes) : task_id(src_task.id), interval(interval), algo_notes(algo_notes) {}