#include "interactive_ui.h"
#include "console_io.h"
#include "../utils/time_utils.h"

#include <iostream>

namespace {

std::optional<TimeInterval> read_interval(bool with_date)
{
    int year = 0, mon = 0, day = 0;
    if (with_date) {
        auto [y, m, d] = read_date("Date (DD.MM.YYYY): ");
        year = y; mon = m; day = d;
    }

    auto [sh, sm] = read_hhmm("Start (HH:MM): ");
    auto [eh, em] = read_hhmm("End (HH:MM, 00:00 = end of day): ");

    const bool end_is_midnight = (eh == 0 && em == 0);

    if (!end_is_midnight && sh * 60 + sm >= eh * 60 + em) {
        std::cout << "  Неправильно введені дані:"
                     " час початку має бути раніше часу кінця.\n";
        return std::nullopt;
    }

    if (with_date) {
        auto start_pt = make_time_point(year, mon, day, sh, sm);
        auto end_pt   = end_is_midnight
            ? make_time_point(year, mon, day + 1, 0, 0)
            : make_time_point(year, mon, day, eh, em);
        return TimeInterval{start_pt, end_pt};
    } else {
        auto start_pt = make_time_point_today(0, sh, sm);
        auto end_pt   = end_is_midnight
            ? make_time_point_today(1, 0, 0)
            : make_time_point_today(0, eh, em);
        return TimeInterval{start_pt, end_pt};
    }
}

} // anonymous namespace

std::optional<TimeBlock> read_timeblock_from_input()
{
    const bool is_repeatable = read_yesno("Repeatable? (y/n): ");

    bool is_every_day = false;
    std::chrono::weekday day_of_week{0};

    if (is_repeatable) {
        is_every_day = read_yesno("Every day? (y/n): ");
        if (!is_every_day)
            day_of_week = read_weekday();
    }

    auto maybe_iv = read_interval(/*with_date=*/!is_repeatable);
    if (!maybe_iv.has_value())
        return std::nullopt;

    return TimeBlock{is_repeatable, is_every_day, *maybe_iv, day_of_week};
}

Task read_task_from_input(uint64_t id)
{
    Task task{};
    task.id = id;

    std::cout << "── New Task (ID " << task.id << ") ──\n";

    task.name = read_line("Name: ");
    task.description = read_line("Description (or Enter to skip): ");

    if (read_yesno("Has deadline? (y/n): ")) {
        auto [year, mon, day] = read_date("Deadline date (DD.MM.YYYY): ");
        auto [dh, dm] = read_hhmm("Deadline time (HH:MM): ");
        task.deadline = make_time_point(year, mon, day, dh, dm);
    }

    task.priority = read_int("Priority (0 = lowest): ");

    int total_min = read_int("Total work duration (minutes): ");
    task.total_duration = std::chrono::minutes{total_min};

    int session_min = read_int_default("Single session duration (minutes, default 25): ", 25);
    task.work_session_duration = std::chrono::minutes{
        session_min > 0 ? session_min : 25};

    int break_min = read_int_default("Break duration (minutes, default 5): ", 5);
    task.break_duration = std::chrono::minutes{
        break_min > 0 ? break_min : 5};

    bool allow_shrink = read_yesno("Allow session shortening? (y/n): ");
    if (allow_shrink) {
        int min_min = read_int("Minimum session duration (minutes): ");
        if (min_min > 0)
            task.min_session_duration = std::chrono::minutes{min_min};
    }

    return task;
}

bool edit_task_interactive(Task& t)
{
    int action = 0;
    do {
        std::cout << "\n";
        std::cout << "  ── Редагування \"" << t.name << "\" ──\n";
        std::cout << "  1. Name:       " << t.name << "\n";
        std::cout << "  2. Description: " << (t.description.empty() ? "(none)" : t.description) << "\n";
        std::cout << "  3. Deadline:    " << (t.deadline.has_value() ? "yes" : "no") << "\n";
        std::cout << "  4. Priority:    " << t.priority << "\n";
        std::cout << "  5. Duration:    " << t.total_duration.count() << " min\n";
        std::cout << "  6. Session:     " << t.work_session_duration.count() << " min\n";
        std::cout << "  7. Break:       " << t.break_duration.count() << " min\n";
        std::cout << "  8. Shortening:  " << (t.min_session_duration.has_value() ? "yes" : "no") << "\n";
        std::cout << "  9. Delete this Task\n";
        std::cout << "  0. Save and exit\n";
        action = read_int("What to change: ");

        switch (action) {
            case 1:
                t.name = read_line("New name: ");
                break;
            case 2:
                t.description = read_line("New description: ");
                break;
            case 3:
                if (read_yesno("Has deadline? (y/n): ")) {
                    auto [year, mon, day] = read_date("Deadline date (DD.MM.YYYY): ");
                    auto [dh, dm] = read_hhmm("Deadline time (HH:MM): ");
                    t.deadline = make_time_point(year, mon, day, dh, dm);
                } else {
                    t.deadline = std::nullopt;
                }
                break;
            case 4:
                t.priority = read_int("New priority: ");
                break;
            case 5: {
                int m = read_int("New duration (minutes): ");
                t.total_duration = std::chrono::minutes{m};
                break;
            }
            case 6: {
                int default_val = t.work_session_duration.count();
                std::string prompt = "New session duration (minutes, default " + std::to_string(default_val) + "): ";
                int m = read_int_default(prompt.c_str(), default_val);
                t.work_session_duration = std::chrono::minutes{m > 0 ? m : 25};
                break;
            }
            case 7: {
                int default_val = t.break_duration.count();
                std::string prompt = "New break duration (minutes, default " + std::to_string(default_val) + "): ";
                int m = read_int_default(prompt.c_str(), default_val);
                t.break_duration = std::chrono::minutes{m > 0 ? m : 5};
                break;
            }
            case 8: {
                bool allow_shrink = read_yesno("Allow shortening? (y/n): ");
                if (allow_shrink) {
                    int mm = read_int("Minimum session duration (minutes): ");
                    t.min_session_duration = mm > 0
                        ? std::optional{std::chrono::minutes{mm}}
                        : std::nullopt;
                } else {
                    t.min_session_duration = std::nullopt;
                }
                break;
            }
            case 9:
                return true;  // signal: delete this task
            default:
                break;
        }
    } while (action != 0);

    return false;  // not deleted
}

void print_timeblocks(const std::vector<TimeBlock>& time_blocks)
{
    if (time_blocks.empty()) {
        std::cout << "  TimeBlock list is empty.\n";
        return;
    }
    auto now = std::chrono::floor<std::chrono::minutes>(std::chrono::system_clock::now());
    for (std::size_t i = 0; i < time_blocks.size(); ++i) {
        TimeInterval iv = time_blocks[i].get_interval(now);
        std::cout << "  #" << (i + 1)
                  << "  [" << fmt_hhmm(iv.start)
                  << " — " << fmt_hhmm(iv.end) << "]";
        if (time_blocks[i].get_repeatable()) {
            std::cout << "  repeat";
            if (time_blocks[i].get_every_day())
                std::cout << "=daily";
            else
                std::cout << "=" << weekday_name(time_blocks[i].get_day_of_week());
        } else {
            std::cout << "  one-time";
        }
        std::cout << "\n";
    }
}

void print_tasks_summary(const std::vector<Task>& tasks)
{
    if (tasks.empty()) {
        std::cout << "  Task list is empty.\n";
        return;
    }
    for (const auto& t : tasks) {
        std::cout << "  #" << t.id << "  \"" << t.name << "\""
                  << "  priority=" << t.priority
                  << "  duration=" << t.total_duration.count() << "min"
                  << "  session=" << t.work_session_duration.count() << "min"
                  << "  break=" << t.break_duration.count() << "min";
        if (t.deadline.has_value()) {
            std::cout << "  deadline=yes";
        } else {
            std::cout << "  deadline=no";
        }
        std::cout << "\n";
    }
}

void print_tasks_short(const std::vector<Task>& tasks)
{
    if (tasks.empty()) {
        std::cout << "  Task list is empty.\n";
        return;
    }
    for (std::size_t i = 0; i < tasks.size(); ++i) {
        std::cout << "  #" << (i + 1)
                  << "  id=" << tasks[i].id
                  << "  \"" << tasks[i].name << "\"\n";
    }
}

void print_tasks_with_remaining_time(const std::vector<Task>& tasks)
{
    if (tasks.empty()) {
        std::cout << "  Task list is empty.\n";
        return;
    }
    for (std::size_t i = 0; i < tasks.size(); ++i) {
        std::cout << "  #" << (i + 1)
                  << "  id=" << tasks[i].id
                  << "  \"" << tasks[i].name << "\" "
                  << "(" << tasks[i].total_duration.count() << " min remaining)\n";
    }
}
