#ifndef TIME_BLOCKS_H
#define TIME_BLOCKS_H

#include <string>
#include <chrono>

using PointInTime = std::chrono::system_clock::time_point; // можна замінити на sys_seconds

struct TimeInterval {
    PointInTime start;
    PointInTime end;
};

struct TimeBlock {
public:
    // Returns the time during which work is prohibited.
    TimeInterval get_interval(PointInTime cursor_time);

    // TODO: Add an exception to the constructor in case the start and end dates in the range are different
    TimeBlock(TimeBlock&&) noexcept = default;
    TimeBlock(bool is_repeatable_flag, bool is_every_day_flag, TimeInterval interval_value, std::chrono::weekday day_of_week_value);
protected:
    bool is_repeatable;
    bool is_every_day;

    // The interval may be ZERO (from zero to zero). Handle this in the algorithm if necessary (for optimization, compare only `.end` with `PointInTime{}`).
    // Then replace with std::optional<TimeInterval>
    TimeInterval interval; // It must be valid for only one day (date). It does not take seconds into account.
    std::chrono::weekday day_of_week; // For repeatable blocks.
private:
    TimeInterval apply_cursor_date_to_interval(PointInTime cursor_time) const;
};

#endif // TIME_BLOCKS_H