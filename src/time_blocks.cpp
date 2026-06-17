#include "time_blocks.h"

#include <chrono>

TimeInterval TimeBlock::get_interval(PointInTime cursor_time) const {
    if (is_repeatable) {
        if (is_every_day) {
            // Повторення щодня
            return apply_cursor_date_to_interval(cursor_time);
        } else {
            // Повторення кожного тижня в один єдиний день
            auto zone = std::chrono::current_zone();
            auto local_cursor = std::chrono::zoned_time{zone, cursor_time}.get_local_time();
            auto cursor_days = std::chrono::floor<std::chrono::days>(local_cursor);
            std::chrono::weekday wd{cursor_days};
            
            if (wd == day_of_week) {
                return apply_cursor_date_to_interval(cursor_time);
            } else {
                return TimeInterval{};
            }
        }
    } else {
        // "Одноразовий" інтервал
        return interval;
    }
}

TimeBlock::TimeBlock(bool is_repeatable_flag, bool is_every_day_flag, TimeInterval interval_value, std::chrono::weekday day_of_week_value)
{
    if (interval_value.start > interval_value.end) {
        throw std::invalid_argument("Start time must be less than or equal to end time.");
    }

    is_repeatable = is_repeatable_flag;
    is_every_day = is_every_day_flag;
    interval = interval_value;
    day_of_week = day_of_week_value;
}

TimeInterval TimeBlock::apply_cursor_date_to_interval(PointInTime cursor_time) const
{
    auto zone = std::chrono::current_zone();
    
    auto local_cursor = std::chrono::zoned_time{zone, cursor_time}.get_local_time();
    auto cursor_days  = std::chrono::floor<std::chrono::days>(local_cursor);
    
    auto local_start = std::chrono::zoned_time{zone, interval.start}.get_local_time();
    auto start_days  = std::chrono::floor<std::chrono::days>(local_start);
    
    auto days_diff = cursor_days - start_days;
    
    auto new_local_start = local_start + days_diff;
    auto new_local_end   = std::chrono::zoned_time{zone, interval.end}.get_local_time() + days_diff;

    auto to_sys = [zone](auto local_time) -> PointInTime {
        try {
            return std::chrono::floor<std::chrono::minutes>(
                std::chrono::zoned_time{zone, local_time, std::chrono::choose::earliest}.get_sys_time());
        } catch (const std::chrono::nonexistent_local_time&) {
            return std::chrono::floor<std::chrono::minutes>(
                std::chrono::zoned_time{zone, local_time + std::chrono::hours{1}, std::chrono::choose::earliest}.get_sys_time());
        }
    };

    return TimeInterval{to_sys(new_local_start), to_sys(new_local_end)};
}


