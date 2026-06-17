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
        std::cout << "Дані успішно завантажено (TimeBlocks: " << time_blocks.size() 
                  << ", Tasks: " << tasks.size() << ").\n";
    }

    int choice = 0;
    do {
        std::cout << "\n";
        std::cout << "──────────── ГОЛОВНЕ МЕНЮ ────────────\n";
        std::cout << "1. Дії з TimeBlock\n";
        std::cout << "2. Дії з Task\n";
        std::cout << "3. Згенерувати розклад\n";
        std::cout << "4. Зберегти дані\n";
        std::cout << "0. Вихід\n";
        choice = read_int("Вибір: ");

        switch (choice) {
            case 1: {
                int tb_choice = 0;
                do {
                    std::cout << "\n";
                    std::cout << "── Меню TimeBlock ──\n";
                    std::cout << "1. Додати TimeBlock\n";
                    std::cout << "2. Показати TimeBlocks\n";
                    std::cout << "3. Редагувати / Видалити TimeBlock\n";
                    std::cout << "0. Повернутися в головне меню\n";
                    tb_choice = read_int("Вибір: ");

                    switch (tb_choice) {
                        case 1: {
                            auto maybe_tb = read_timeblock_from_input();
                            if (maybe_tb.has_value()) {
                                try {
                                    time_blocks.push_back(std::move(*maybe_tb));
                                    std::cout << "  TimeBlock #" << time_blocks.size() << " додано.\n";
                                } catch (const std::exception& ex) {
                                    std::cout << "  Помилка конструктора: " << ex.what() << '\n';
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
                                std::cout << "  Список TimeBlocks порожній.\n";
                                break;
                            }

                            print_timeblocks(time_blocks);

                            int tb_idx = read_int("Номер TimeBlock (або 0 для скасування): ");
                            if (tb_idx <= 0 || tb_idx > static_cast<int>(time_blocks.size())) {
                                if (tb_idx != 0)
                                    std::cout << "  Неправильний номер.\n";
                                break;
                            }
                            --tb_idx;

                            std::cout << "  1. Перестворити (ввести всі поля заново)\n";
                            std::cout << "  2. Видалити\n";
                            std::cout << "  0. Скасувати\n";
                            int tb_action = read_int("Дія: ");

                            if (tb_action == 2) {
                                time_blocks.erase(time_blocks.begin() + tb_idx);
                                std::cout << "  TimeBlock видалено.\n";
                            } else if (tb_action == 1) {
                                auto maybe_tb = read_timeblock_from_input();
                                if (maybe_tb.has_value()) {
                                    try {
                                        time_blocks[tb_idx] = std::move(*maybe_tb);
                                        std::cout << "  TimeBlock #" << (tb_idx + 1) << " оновлено.\n";
                                    } catch (const std::exception& ex) {
                                        std::cout << "  Помилка: " << ex.what() << '\n';
                                    }
                                }
                            }
                            break;
                        }
                        case 0:
                            break;
                        default:
                            std::cout << "Неправильний вибір.\n";
                            break;
                    }
                } while (tb_choice != 0);
                break;
            }

            case 2: {
                int t_choice = 0;
                do {
                    std::cout << "\n";
                    std::cout << "── Меню Task ──\n";
                    std::cout << "1. Додати Task\n";
                    std::cout << "2. Показати Tasks\n";
                    std::cout << "3. Редагувати / Видалити Task\n";
                    std::cout << "4. Відмітити прогрес (витрачений час)\n";
                    std::cout << "5. Детальна інформація про Task (з описом)\n";
                    std::cout << "0. Повернутися в головне меню\n";
                    t_choice = read_int("Вибір: ");

                    switch (t_choice) {
                        case 1: {
                            Task task = read_task_from_input(next_task_id++);
                            std::cout << "  Task #" << task.id << " \"" << task.name << "\" додано.\n";
                            tasks.push_back(std::move(task));
                            break;
                        }
                        case 2: {
                            print_tasks_summary(tasks);
                            break;
                        }
                        case 3: {
                            if (tasks.empty()) {
                                std::cout << "  Список завдань порожній.\n";
                                break;
                            }

                            print_tasks_short(tasks);

                            int t_idx = read_int("Номер Task (або 0 для скасування): ");
                            if (t_idx <= 0 || t_idx > static_cast<int>(tasks.size())) {
                                if (t_idx != 0)
                                    std::cout << "  Неправильний номер.\n";
                                break;
                            }
                            --t_idx;

                            bool deleted = edit_task_interactive(tasks[t_idx]);
                            if (deleted) {
                                tasks.erase(tasks.begin() + t_idx);
                                std::cout << "  Task видалено.\n";
                            }
                            break;
                        }
                        case 4: {
                            if (tasks.empty()) {
                                std::cout << "  Список завдань порожній.\n";
                                break;
                            }

                            print_tasks_with_remaining_time(tasks);

                            int t_idx = read_int("Номер Task (або 0 для скасування): ");
                            if (t_idx <= 0 || t_idx > static_cast<int>(tasks.size())) {
                                if (t_idx != 0)
                                    std::cout << "  Неправильний номер.\n";
                                break;
                            }
                            --t_idx;

                            int default_dur = static_cast<int>(tasks[t_idx].work_session_duration.count());
                            std::string prompt = "Витрачено хвилин (за замовчуванням " + std::to_string(default_dur) + "): ";
                            int spent = read_int_default(prompt.c_str(), default_dur);

                            tasks[t_idx].total_duration -= std::chrono::minutes{spent};
                            
                            if (tasks[t_idx].total_duration.count() <= 0) {
                                std::cout << "  Віднято " << spent << " хв.\n";
                                std::cout << "  Завдання повністю виконано!\n";
#if SAVE_COMPLETED_TASKS
                                append_completed_task(tasks[t_idx]);
#endif
                                tasks.erase(tasks.begin() + t_idx);
                            } else {
                                std::cout << "  Віднято " << spent << " хв. Залишилось: " << tasks[t_idx].total_duration.count() << " хв.\n";
                            }
                            break;
                        }
                        case 5: {
                            if (tasks.empty()) {
                                std::cout << "  Список завдань порожній.\n";
                                break;
                            }

                            print_tasks_short(tasks);

                            int t_idx = read_int("Номер Task (або 0 для скасування): ");
                            if (t_idx <= 0 || t_idx > static_cast<int>(tasks.size())) {
                                if (t_idx != 0)
                                    std::cout << "  Неправильний номер.\n";
                                break;
                            }
                            --t_idx;

                            const auto& t = tasks[t_idx];
                            std::cout << "\n  ── Детальна інформація ──\n";
                            std::cout << "  Назва: " << t.name << "\n";
                            std::cout << "  Опис:  " << (t.description.empty() ? "(немає опису)" : t.description) << "\n";
                            std::cout << "  Дедлайн: ";
                            if (t.deadline.has_value()) {
                                std::cout << fmt_datetime(*t.deadline) << "\n";
                            } else {
                                std::cout << "немає\n";
                            }
                            std::cout << "  Пріоритет: " << t.priority << "\n";
                            std::cout << "  Тривалість: " << t.total_duration.count() << " хв\n";
                            std::cout << "  Сесія: " << t.work_session_duration.count() << " хв\n";
                            std::cout << "  Перерва: " << t.break_duration.count() << " хв\n";
                            std::cout << "  Скорочення: " << (t.min_session_duration.has_value() ? "дозволено" : "заборонено") << "\n";
                            if (t.min_session_duration.has_value()) {
                                std::cout << "  Мін. сесія: " << t.min_session_duration->count() << " хв\n";
                            }
                            std::cout << "  ─────────────────────────\n";
                            break;
                        }
                        case 0:
                            break;
                        default:
                            std::cout << "Неправильний вибір.\n";
                            break;
                    }
                } while (t_choice != 0);
                break;
            }

            case 3: {
                if (tasks.empty()) {
                    std::cout << "  Спочатку додайте хоча б одне завдання.\n";
                    break;
                }

                std::cout << "  Генерація розкладу...\n";
                auto result = get_schedule(time_blocks, tasks);
                printSchedule(result);
                break;
            }

            case 4: {
                if (save_data(time_blocks, tasks)) {
                    std::cout << "  Дані успішно збережено.\n";
                }
                break;
            }

            case 0:
                std::cout << "  Збереження даних перед виходом...\n";
                if (save_data(time_blocks, tasks)) {
                    std::cout << "  Дані успішно збережено.\n";
                }
                break;
            default:
                std::cout << "Неправильний вибір. Спробуйте ще раз.\n";
                break;
        }
    } while (choice != 0);
    return 0;
}