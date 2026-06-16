#ifndef TIME_BLOCKS_H
#define TIME_BLOCKS_H

#include <string>
#include <chrono>

using PointInTime = std::chrono::sys_time<std::chrono::minutes>;

struct TimeInterval {
    PointInTime start;
    PointInTime end;
};

struct TimeBlock {
public:
    TimeInterval get_interval(PointInTime cursor_time); // Returns the time during which work is prohibited

    bool get_repeatable() const { return is_repeatable; }
    bool get_every_day() const { return is_every_day; }
    std::chrono::weekday get_day_of_week() const { return day_of_week; }
    const TimeInterval& get_raw_interval() const { return interval; }

    TimeBlock(const TimeBlock&) = default;
    TimeBlock(TimeBlock&&) noexcept = default;
    TimeBlock& operator=(const TimeBlock&) = default;
    TimeBlock& operator=(TimeBlock&&) noexcept = default;
    TimeBlock(bool is_repeatable_flag, bool is_every_day_flag, TimeInterval interval_value, std::chrono::weekday day_of_week_value);
protected:
    bool is_repeatable;
    bool is_every_day;

    TimeInterval interval; // It must be valid for only one day (date). It does not take seconds into account.
    std::chrono::weekday day_of_week; // For repeatable blocks.
private:
    TimeInterval apply_cursor_date_to_interval(PointInTime cursor_time) const;
};

#endif // TIME_BLOCKS_H