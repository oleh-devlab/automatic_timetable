# automatic_timetable

Status: Currently in the planning and development stages for a minimum viable product.

# Description

The project is designed to solve a scheduling problem that involves a large number of constraints for each task and time slot.
For example, you provide the program with a to-do list and answers to certain questions (such as relative priority, estimated time required, the importance of breaking tasks into smaller time blocks, and so on). You also specify the times when you are unavailable to work. The program generates a schedule for completing these tasks, taking into account the provided constraints and certain productivity techniques (e.g., Pomodoro, interval work).

# Technical Implementation Plan

C++ has been chosen as the primary programming language. We plan to use the built-in `<chrono>` library for time-related operations. For now, all necessary data will be stored in regular files rather than a database.