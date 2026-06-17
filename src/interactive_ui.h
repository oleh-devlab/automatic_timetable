#ifndef INTERACTIVE_UI_H
#define INTERACTIVE_UI_H

#include "time_blocks.h"
#include "task.h"
#include <optional>
#include <cstdint>
#include <vector>

std::optional<TimeBlock> read_timeblock_from_input();
Task read_task_from_input(uint64_t id);
bool edit_task_interactive(Task& t);

void print_timeblocks(const std::vector<TimeBlock>& time_blocks);
void print_tasks_summary(const std::vector<Task>& tasks);
void print_tasks_short(const std::vector<Task>& tasks);
void print_tasks_with_remaining_time(const std::vector<Task>& tasks);

#endif
