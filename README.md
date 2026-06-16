# automatic_timetable

Status: We are currently developing a minimal interface for using the algorithm

# Description

The project is designed to solve a scheduling problem that involves a large number of constraints for each task and time slot.
For example, you provide the program with a to-do list and answers to certain questions (such as relative priority, estimated time required, the importance of breaking tasks into smaller time blocks, and so on). You also specify the times when you are unavailable to work. The program generates a schedule for completing these tasks, taking into account the provided constraints and certain productivity techniques (e.g., Pomodoro, interval work).

# Technical Implementation

C++ has been chosen as the primary programming language. We plan to use the built-in `<chrono>` library for time-related operations. For now, all necessary data will be stored in regular files rather than in a database, in order to eliminate external dependencies.

This project was custom-designed manually (System Design, class hierarchy, algorithmic approach). The boilerplate code and UI implementation were accelerated using AI tools, followed by manual code review and optimization (e.g., memory management via windowing approach).

# Algorithm

[All versions of the algorithm for creating a schedule](docs/algorithms.md).
