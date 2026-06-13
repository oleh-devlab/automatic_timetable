#include <iostream>
#include <chrono>
#include <limits>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

#include "schedule_control.h"
#include "time_blocks.h"

// ─── helpers ─────────────────────────────────────────────────────────────────

namespace {

// Builds a time_point for the given calendar date + time (UTC).
// TODO: (std::chrono::current_zone)
PointInTime make_time_point(int year, int month, int day, int hours, int minutes)
{
    std::chrono::year_month_day ymd{
        std::chrono::year{year},
        std::chrono::month{static_cast<unsigned>(month)},
        std::chrono::day{static_cast<unsigned>(day)}
    };
    return std::chrono::sys_days{ymd}
         + std::chrono::hours(hours)
         + std::chrono::minutes(minutes);
}

// Builds a time_point for today (UTC) at h:m — used for repeatable blocks.
// TODO:  (std::chrono::current_zone)
PointInTime make_time_point_today(int hours, int minutes)
{
    auto today = std::chrono::floor<std::chrono::days>(
        std::chrono::system_clock::now());
    return today
         + std::chrono::hours(hours)
         + std::chrono::minutes(minutes);
}

// Formats a PointInTime as "HH:MM" (UTC).
std::string fmt_hhmm(PointInTime tp)
{
    auto dp = std::chrono::floor<std::chrono::days>(tp);
    auto hh = std::chrono::floor<std::chrono::hours>(tp - dp);
    auto mm = std::chrono::floor<std::chrono::minutes>(tp - dp - hh);
    char buf[8];
    std::snprintf(buf, sizeof(buf), "%02d:%02d",
                  static_cast<int>(hh.count()),
                  static_cast<int>(mm.count()));
    return buf;
}

// Reads "HH:MM" from stdin; loops until valid.
std::pair<int, int> read_hhmm(const char* prompt)
{
    for (;;) {
        std::cout << prompt;
        int  h = -1, m = -1;
        char sep = 0;
        std::cin >> h >> sep >> m;
        if (std::cin && sep == ':' && h >= 0 && h <= 23 && m >= 0 && m <= 59)
            return {h, m};
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "  Неправильний формат — введіть HH:MM (наприклад, 09:30).\n";
    }
}

// Reads "DD.MM.YYYY" from stdin; loops until valid calendar date.
std::tuple<int, int, int> read_date(const char* prompt)
{
    for (;;) {
        std::cout << prompt;
        int  d = -1, mon = -1, y = -1;
        char s1 = 0, s2 = 0;
        std::cin >> d >> s1 >> mon >> s2 >> y;
        if (std::cin && s1 == '.' && s2 == '.') {
            std::chrono::year_month_day ymd{
                std::chrono::year{y},
                std::chrono::month{static_cast<unsigned>(mon)},
                std::chrono::day{static_cast<unsigned>(d)}
            };
            if (ymd.ok()) return {y, mon, d};
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "  Неправильний формат — введіть ДД.ММ.РРРР"
                     " (наприклад, 25.12.2025).\n";
    }
}

// Reads y/n answer from stdin.
bool read_yesno(const char* prompt)
{
    for (;;) {
        char c = 0;
        std::cout << prompt;
        std::cin >> c;
        if (c == 'y' || c == 'Y') return true;
        if (c == 'n' || c == 'N') return false;
        std::cout << "  Введіть y або n.\n";
    }
}

// Reads a weekday number 0–6 (Sun–Sat) from stdin.
std::chrono::weekday read_weekday()
{
    for (;;) {
        std::cout << "  День тижня"
                     " (0=Нд, 1=Пн, 2=Вт, 3=Ср, 4=Чт, 5=Пт, 6=Сб): ";
        unsigned d = 7;
        std::cin >> d;
        if (std::cin && d <= 6) return std::chrono::weekday{d};
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "  Введіть число від 0 до 6.\n";
    }
}

void flush_line()
{
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

} // anonymous namespace

// ─── main ────────────────────────────────────────────────────────────────────

int main() {
#if defined(_WIN32) || defined(_WIN64)
    SetConsoleOutputCP(CP_UTF8);
#endif

    std::vector<TimeBlock> time_blocks;

    int choice = 0;
    do {
        std::cout << "1. Write TimeBlock" << std::endl;
        std::cout << "2. Read TimeBlocks" << std::endl;
        std::cout << "0. Exit" << std::endl;
        std::cin >> choice;
        flush_line();

        switch (choice) {
            case 1: {
                // Write time block
                const bool is_repeatable = read_yesno("Повторюваний? (y/n): ");

                bool is_every_day = false;
                std::chrono::weekday day_of_week{0};

                if (is_repeatable) {
                    is_every_day = read_yesno("Кожного дня? (y/n): ");
                    if (!is_every_day)
                        day_of_week = read_weekday();
                }

                TimeInterval interval{};

                if (!is_repeatable) {
                    auto [year, mon, day] = read_date("Дата (ДД.ММ.РРРР): ");
                    auto [sh, sm] = read_hhmm("Початок (HH:MM): ");
                    auto [eh, em] = read_hhmm("Кінець  (HH:MM): ");

                    if (sh * 60 + sm >= eh * 60 + em) {
                        std::cout << "  Неправильно введені дані:"
                                     " час початку має бути раніше часу кінця.\n";
                        break;
                    }

                    interval = { make_time_point(year, mon, day, sh, sm),
                                 make_time_point(year, mon, day, eh, em) };
                } else {
                    auto [sh, sm] = read_hhmm("Початок (HH:MM): ");
                    auto [eh, em] = read_hhmm("Кінець  (HH:MM): ");

                    if (sh * 60 + sm >= eh * 60 + em) {
                        std::cout << "  Неправильно введені дані:"
                                     " час початку має бути раніше часу кінця.\n";
                        break;
                    }

                    interval = { make_time_point_today(sh, sm),
                                 make_time_point_today(eh, em) };
                }

                try {
                    time_blocks.push_back(
                        TimeBlock{is_repeatable, is_every_day, interval, day_of_week});
                    std::cout << "  TimeBlock #" << time_blocks.size() << " додано.\n";
                } catch (const std::exception& ex) {
                    std::cout << "  Помилка конструктора: " << ex.what() << '\n';
                }

                break;
            }
            case 2: {
                // Read time blocks
                if (time_blocks.empty()) {
                    std::cout << "  Список порожній.\n";
                    break;
                }
                
                auto now = std::chrono::system_clock::now();
                for (std::size_t i = 0; i < time_blocks.size(); ++i) {
                    TimeInterval iv = time_blocks[i].get_interval(now);
                    std::cout << "  #" << (i + 1)
                              << "  [" << fmt_hhmm(iv.start)
                              << " — " << fmt_hhmm(iv.end) << "]\n";
                }
                break;
            }
            case 0: {
                break;
            }
            default: {
                std::cout << "Wrong choice. Try again." << std::endl;
                break;
            }
        }
    } while (choice != 0);
    // TODO: timepoint in my timezone from interface

    return 0;
}