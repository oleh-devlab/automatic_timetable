# All versions of the algorithms for creating a schedule
## First simple version
- Minimal interactivity.
- Designed to demonstrate ideas and build a base for heavy algorithms.

```
1. Sorts the array of tasks in ascending order by: deadlines (to the nearest day), and priorities.
2. Initializes per-task progress (TaskProgress), calculating session count from total_duration / work_session_duration.
2.1. Initializes the "cursor" with the current system time.
2.2. Expands repeatable TimeBlocks into concrete intervals using a sliding window of 7 days (expand_time_blocks + merge_intervals). When the cursor reaches the end of the current window, the next 7-day chunk is generated.
2.3. Searches for the nearest occupied block from the future relative to the cursor. Determines the free window [cursor, free_end).
2.4. Iterates through tasks in sorted order. If the next session fits into the free window, inserts it as a FinalBlock, inserts a break, and advances the cursor. If the break does not fit before the next occupied block, checks whether the block itself can serve as a break (block_dur >= break_dur - 5min).
2.4.1. If the session doesn't fit, tries the next task (backfilling).
2.4.2. Before inserting, checks whether the session would exceed the task's deadline.
2.4. (`exception_handling`). If the deadline is exceeded:
- (TODO) We could check if there are any tasks inserted before A that can be moved so that they come after task (deadline) A.
- (Implemented) Reduce the session duration down to min_session_duration (if set) and insert the shortened session.
- (Implemented, fallback) Add a note to this FinalBlock and skip remaining sessions of this task.
2.5. Moves the cursor to the end of the current block (+1 minute) and repeats steps 2.2–2.4 until all tasks are completed.
If the last block in the schedule is a break, it is removed.
```
 
## Plans
- Use a genetic algorithm.