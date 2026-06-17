#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include "../core/time_blocks.h"
#include "../core/task.h"

#include <vector>
#include <string>
#include <cstdint>
#include <ostream>

// --- Save / Load Data ------------------------------------------------

// Sets custom paths for the data files. If a parameter is empty, the default is kept.
void set_file_paths(const std::string& tasks_file,
                    const std::string& time_blocks_file,
                    const std::string& completed_tasks_file);

// Saves tasks and time blocks to TSV files (tasks.tsv, time_blocks.tsv).
// Returns true if both saved successfully.
bool save_data(const std::vector<TimeBlock>& tbs,
               const std::vector<Task>& tasks);

// Loads tasks and time blocks from TSV files.
// Populates the vectors. Also updates next_task_id to be max(id) + 1.
// Returns true if loaded successfully (or if files don't exist yet).
bool load_data(std::vector<TimeBlock>& tbs,
               std::vector<Task>& tasks,
               uint64_t& next_task_id);

// Appends a completed task to completed_tasks.tsv
void append_completed_task(const Task& t);

// Writes schedule to a TSV format
void write_schedule_tsv(std::ostream& out, const std::vector<FinalBlock>& schedule);

#endif // PERSISTENCE_H
