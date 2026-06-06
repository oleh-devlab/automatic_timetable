# automatic_timetable

Status: Currently in the planning and development stages for a minimum viable product.

# Description

The project is designed to solve a scheduling problem that involves a large number of constraints for each task and time slot.
For example, you provide the program with a to-do list and answers to certain questions (such as relative priority, estimated time required, the importance of breaking tasks into smaller time blocks, and so on). You also specify the times when you are unavailable to work. The program generates a schedule for completing these tasks, taking into account the provided constraints and certain productivity techniques (e.g., Pomodoro, interval work).

# Technical Implementation Plan

C++ has been chosen as the primary programming language. We plan to use the built-in `<chrono>` library for time-related operations. For now, all necessary data will be stored in regular files rather than a database.

# Algorithm
## First simple version (minimal interactivity) - not yet implemented
```
1. Sorts (using a stable sort or specific conditions) the array of tasks in ascending order by: deadlines (to the nearest day—i.e., disregarding hours and minutes), and priorities.
2. Converts the sorted tasks into an array of final tasks, taking the Pomodoro into account (the algorithm will insert breaks later based on an analysis of the work block duration and the number of past blocks)
2.1. Initializes the “cursor” with the current system time (or a user-specified time).
2.2. Reads the array of time blocks, searches among them for the block closest to the current cursor position from the future (or the present, but not the past).
2.3. Begins iterating through the tasks.
2.4. If the execution time for the first part of the task fits into the free time, it inserts it, inserts a break (also a FinalBlock with the is_task flag set to False), moves the cursor to the end of the break, and repeats step 2.2, attempting to insert the next part of the task. It is necessary to decide what to do if the break does not fit before the occupied block (it would be good to mark time blocks with flags indicating that the entire block is essentially a substitute for a break. Also check whether the block size is greater than or equal to (to the nearest 5 minutes) the break (i.e., whether it is greater than (break - 5 minutes)))
2.4.1. Otherwise, check whether the subsequent tasks fit according to step 2.4. If not, skip this free block.
2.4.2. Before inserting, check whether this will cause the exact deadline time to be exceeded.
2.4. (`exception_handling`). If the deadline is exceeded or if it does not fit into a free block, I haven't figured out what to do yet. Let A be a task that we can no longer insert because we would exceed the deadline. There are several options:
- We could check if there are any tasks inserted before A that can be moved so that they come after task (deadline) A.
- We could slightly reduce the time for the subtasks of task A (according to the Task settings) and try to reinsert these subtasks.
- However, for now, to simplify the creation of the initial version, the algorithm will simply add a note about this issue to this FinalBlock, remove the subsequent Pomodoro blocks for this task, and continue working on the remaining tasks.
2.4. Moves the cursor to the end of the current block (+1 minute, for example) and repeats steps 2.2–2.4 until all tasks are completed.
```