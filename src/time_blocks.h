#ifndef TIME_BLOCKS_H
#define TIME_BLOCKS_H

#include <string>
#include <chrono>

struct TimeInterval {
    std::chrono::steady_clock::time_point start;
    std::chrono::steady_clock::time_point end;
};

struct TimeBlock {
public:
    // Returns the time during which work is prohibited.
    TimeInterval get_interval(std::chrono::steady_clock::time_point cursor_time);
protected:
    bool repeatable;
    // Settings for saving either a one-time time, or a time for every day or week.
};

#endif // TIME_BLOCKS_H