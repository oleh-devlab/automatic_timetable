#ifndef CONSOLE_IO_H
#define CONSOLE_IO_H

#include <string>
#include <tuple>
#include <utility>
#include <chrono>

void flush_line();
std::pair<int, int> read_hhmm(const char* prompt);
std::tuple<int, int, int> read_date(const char* prompt);
bool read_yesno(const char* prompt);
std::chrono::weekday read_weekday();
int read_int(const char* prompt);
int read_int_default(const char* prompt, int default_val);
std::string read_line(const char* prompt);

#endif
