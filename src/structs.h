#include <string>
#include <chrono>

struct TimeInterval {
    std::chrono::steady_clock::time_point start;
    std::chrono::steady_clock::time_point end;
};

struct TimeBlock {
public:
    // Returns the time during which work is prohibited.
    const TimeInterval get_interval(std::chrono::steady_clock::time_point cursor_time);
protected:
    bool repeatable;
    // Settings for saving either a one-time time, or a time for every day or week.
};

struct Task {
    std::string name;
    std::string description;
    // Additional settings
};

struct FinalTask: public Task {
    std::string algo_notes; // Notes on the algorithm. For example, ...
    // ... it did not increase the runtime because it was unable to generate a schedule with the increase.
    TimeInterval interval;
};