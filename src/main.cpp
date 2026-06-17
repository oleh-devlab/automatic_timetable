#include <iostream>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

#include "schedule_control.h"
#include "interactive_ui.h"
#include "console_io.h"
#include "time_utils.h"
#include "persistence.h"

#define SAVE_COMPLETED_TASKS 1

// ─── main ────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
#if defined(_WIN32) || defined(_WIN64)
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    std::vector<TimeBlock> time_blocks;
    std::vector<Task> tasks;
    uint64_t next_task_id = 1;

    bool loaded = load_data(time_blocks, tasks, next_task_id);

    bool get_schedule_flag = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--get_schedule") {
            get_schedule_flag = true;
        }
    }

    if (get_schedule_flag) {
        auto schedule = get_schedule(time_blocks, tasks);
        write_schedule_tsv(std::cout, schedule);
        return 0;
    }

    if (loaded) {
        std::cout << "Data loaded successfully (TimeBlocks: " << time_blocks.size() 
                  << ", Tasks: " << tasks.size() << ").\n";
    }

    int choice = 0;
    do {
        std::cout << "\n";
        std::cout << "──────────── MAIN MENU ────────────\n";
        std::cout << "1. Manage TimeBlocks\n";
        std::cout << "2. Manage Tasks\n";
        std::cout << "3. Generate Schedule\n";
        std::cout << "4. Save Data\n";
        std::cout << "0. Exit\n";
        choice = read_int("Choice: ");

        switch (choice) {
            case 1: {
                int tb_choice = 0;
                do {
                    std::cout << "\n";
                    std::cout << "── TimeBlock Menu ──\n";
                    std::cout << "1. Add TimeBlock\n";
                    std::cout << "2. View TimeBlocks\n";
                    std::cout << "3. Edit / Delete TimeBlock\n";
                    std::cout << "0. Return to Main Menu\n";
                    tb_choice = read_int("Choice: ");

                    switch (tb_choice) {
                        case 1: {
                            auto maybe_tb = read_timeblock_from_input();
                            if (maybe_tb.has_value()) {
                                try {
                                    time_blocks.push_back(std::move(*maybe_tb));
                                    std::cout << "  TimeBlock #" << time_blocks.size() << " added.\n";
                                } catch (const std::exception& ex) {
                                    std::cout << "  Constructor error: " << ex.what() << '\n';
                                }
                            }
                            break;
                        }
                        case 2: {
                            print_timeblocks(time_blocks);
                            break;
                        }
                        case 3: {
                            if (time_blocks.empty()) {
                                std::cout << "  TimeBlock list is empty.\n";
                                break;
                            }

                            print_timeblocks(time_blocks);

                            int tb_idx = read_int("TimeBlock number (or 0 to cancel): ");
                            if (tb_idx <= 0 || tb_idx > static_cast<int>(time_blocks.size())) {
                                if (tb_idx != 0)
                                    std::cout << "  Invalid number.\n";
                                break;
                            }
                            --tb_idx;

                            std::cout << "  1. Recreate (enter all fields again)\n";
                            std::cout << "  2. Delete\n";
                            std::cout << "  0. Cancel\n";
                            int tb_action = read_int("Action: ");

                            if (tb_action == 2) {
                                time_blocks.erase(time_blocks.begin() + tb_idx);
                                std::cout << "  TimeBlock deleted.\n";
                            } else if (tb_action == 1) {
                                auto maybe_tb = read_timeblock_from_input();
                                if (maybe_tb.has_value()) {
                                    try {
                                        time_blocks[tb_idx] = std::move(*maybe_tb);
                                        std::cout << "  TimeBlock #" << (tb_idx + 1) << " updated.\n";
                                    } catch (const std::exception& ex) {
                                        std::cout << "  Error: " << ex.what() << '\n';
                                    }
                                }
                            }
                            break;
                        }
                        case 0:
                            break;
                        default:
                            std::cout << "Invalid choice.\n";
                            break;
                    }
                } while (tb_choice != 0);
                break;
            }

            case 2: {
                int t_choice = 0;
                do {
                    std::cout << "\n";
                    std::cout << "── Task Menu ──\n";
                    std::cout << "1. Add Task\n";
                    std::cout << "2. View Tasks\n";
                    std::cout << "3. Edit / Delete Task\n";
                    std::cout << "4. Mark progress (time spent)\n";
                    std::cout << "5. Task details (with description)\n";
                    std::cout << "0. Return to Main Menu\n";
                    t_choice = read_int("Choice: ");

                    switch (t_choice) {
                        case 1: {
                            Task task = read_task_from_input(next_task_id++);
                            std::cout << "  Task #" << task.id << " \"" << task.name << "\" added.\n";
                            tasks.push_back(std::move(task));
                            break;
                        }
                        case 2: {
                            print_tasks_summary(tasks);
                            break;
                        }
                        case 3: {
                            if (tasks.empty()) {
                                std::cout << "  Task list is empty.\n";
                                break;
                            }

                            print_tasks_short(tasks);

                            int t_idx = read_int("Task number (or 0 to cancel): ");
                            if (t_idx <= 0 || t_idx > static_cast<int>(tasks.size())) {
                                if (t_idx != 0)
                                    std::cout << "  Invalid number.\n";
                                break;
                            }
                            --t_idx;

                            bool deleted = edit_task_interactive(tasks[t_idx]);
                            if (deleted) {
                                tasks.erase(tasks.begin() + t_idx);
                                std::cout << "  Task deleted.\n";
                            }
                            break;
                        }
                        case 4: {
                            if (tasks.empty()) {
                                std::cout << "  Task list is empty.\n";
                                break;
                            }

                            print_tasks_with_remaining_time(tasks);

                            int t_idx = read_int("Task number (or 0 to cancel): ");
                            if (t_idx <= 0 || t_idx > static_cast<int>(tasks.size())) {
                                if (t_idx != 0)
                                    std::cout << "  Invalid number.\n";
                                break;
                            }
                            --t_idx;

                            int default_dur = static_cast<int>(tasks[t_idx].work_session_duration.count());
                            std::string prompt = "Minutes spent (default " + std::to_string(default_dur) + "): ";
                            int spent = read_int_default(prompt.c_str(), default_dur);

                            tasks[t_idx].total_duration -= std::chrono::minutes{spent};
                            
                            if (tasks[t_idx].total_duration.count() <= 0) {
                                std::cout << "  Subtracted " << spent << " min.\n";
                                std::cout << "  Task fully completed!\n";
#if SAVE_COMPLETED_TASKS
                                append_completed_task(tasks[t_idx]);
#endif
                                tasks.erase(tasks.begin() + t_idx);
                            } else {
                                std::cout << "  Subtracted " << spent << " min. Remaining: " << tasks[t_idx].total_duration.count() << " min.\n";
                            }
                            break;
                        }
                        case 5: {
                            if (tasks.empty()) {
                                std::cout << "  Task list is empty.\n";
                                break;
                            }

                            print_tasks_short(tasks);

                            int t_idx = read_int("Task number (or 0 to cancel): ");
                            if (t_idx <= 0 || t_idx > static_cast<int>(tasks.size())) {
                                if (t_idx != 0)
                                    std::cout << "  Invalid number.\n";
                                break;
                            }
                            --t_idx;

                            const auto& t = tasks[t_idx];
                            std::cout << "\n  ── Detailed Information ──\n";
                            std::cout << "  Name: " << t.name << "\n";
                            std::cout << "  Description: " << (t.description.empty() ? "(no description)" : t.description) << "\n";
                            std::cout << "  Deadline: ";
                            if (t.deadline.has_value()) {
                                std::cout << fmt_datetime(*t.deadline) << "\n";
                            } else {
                                std::cout << "none\n";
                            }
                            std::cout << "  Priority: " << t.priority << "\n";
                            std::cout << "  Duration: " << t.total_duration.count() << " min\n";
                            std::cout << "  Session: " << t.work_session_duration.count() << " min\n";
                            std::cout << "  Break: " << t.break_duration.count() << " min\n";
                            std::cout << "  Shortening: " << (t.min_session_duration.has_value() ? "allowed" : "forbidden") << "\n";
                            if (t.min_session_duration.has_value()) {
                                std::cout << "  Min session: " << t.min_session_duration->count() << " min\n";
                            }
                            std::cout << "  ─────────────────────────\n";
                            break;
                        }
                        case 0:
                            break;
                        default:
                            std::cout << "Invalid choice.\n";
                            break;
                    }
                } while (t_choice != 0);
                break;
            }

            case 3: {
                if (tasks.empty()) {
                    std::cout << "  Please add at least one task first.\n";
                    break;
                }

                std::cout << "  Generating schedule...\n";
                auto result = get_schedule(time_blocks, tasks);
                printSchedule(result);
                break;
            }

            case 4: {
                if (save_data(time_blocks, tasks)) {
                    std::cout << "  Data saved successfully.\n";
                }
                break;
            }

            case 0:
                std::cout << "  Saving data before exiting...\n";
                if (save_data(time_blocks, tasks)) {
                    std::cout << "  Data saved successfully.\n";
                }
                break;
            default:
                std::cout << "Invalid choice. Спробуйте ще раз.\n";
                break;
        }
    } while (choice != 0);
    return 0;
}