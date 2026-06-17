#include "console_io.h"

#include <iostream>
#include <limits>

void flush_line()
{
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

std::pair<int, int> read_hhmm(const char* prompt)
{
    for (;;) {
        std::cout << prompt;
        int  h = -1, m = -1;
        char sep = 0;
        std::cin >> h >> sep >> m;
        if (std::cin && sep == ':' && h >= 0 && h <= 23 && m >= 0 && m <= 59) {
            flush_line();
            return {h, m};
        }
        flush_line();
        std::cout << "  Неправильний формат — введіть HH:MM (наприклад, 09:30).\n";
    }
}

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
            if (ymd.ok()) {
                flush_line();
                return {y, mon, d};
            }
        }
        flush_line();
        std::cout << "  Неправильний формат — введіть ДД.ММ.РРРР"
                     " (наприклад, 25.12.2025).\n";
    }
}

bool read_yesno(const char* prompt)
{
    for (;;) {
        char c = 0;
        std::cout << prompt;
        std::cin >> c;
        if (c == 'y' || c == 'Y') { flush_line(); return true; }
        if (c == 'n' || c == 'N') { flush_line(); return false; }
        std::cout << "  Введіть y або n.\n";
    }
}

std::chrono::weekday read_weekday()
{
    for (;;) {
        std::cout << "  День тижня"
                     " (0=Нд, 1=Пн, 2=Вт, 3=Ср, 4=Чт, 5=Пт, 6=Сб): ";
        unsigned d = 7;
        std::cin >> d;
        if (std::cin && d <= 6) {
            flush_line();
            return std::chrono::weekday{d};
        }
        flush_line();
        std::cout << "  Введіть число від 0 до 6.\n";
    }
}

int read_int(const char* prompt)
{
    for (;;) {
        std::cout << prompt;
        int val = -1;
        std::cin >> val;
        if (std::cin && val >= 0) {
            flush_line();
            return val;
        }
        flush_line();
        std::cout << "  Введіть невід'ємне ціле число.\n";
    }
}

int read_int_default(const char* prompt, int default_val)
{
    for (;;) {
        std::cout << prompt;
        std::string line;
        if (!std::getline(std::cin, line)) {
            std::cin.clear();
            continue;
        }
        
        while (!line.empty() && (line.back() == ' ' || line.back() == '\r')) {
            line.pop_back();
        }
        
        if (line.empty()) {
            return default_val;
        }
        
        try {
            size_t pos = 0;
            int val = std::stoi(line, &pos);
            if (pos == line.size() && val >= 0) {
                return val;
            }
        } catch (...) {
        }
        std::cout << "  Введіть невід'ємне ціле число або просто натисніть Enter.\n";
    }
}

std::string read_line(const char* prompt)
{
    std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    return line;
}
