#include "time_blocks.h"

#include <chrono>

TimeInterval TimeBlock::get_interval(PointInTime cursor_time) {
    if (is_repeatable) {
        if (is_every_day) {
            // Повторення щодня
            return apply_cursor_date_to_interval(cursor_time);
        } else {
            // Повторення кожного тижня в один єдиний день
            std::chrono::weekday cursor_wd = std::chrono::floor<std::chrono::days>(cursor_time);
            
            if (cursor_wd == day_of_week) {
                return apply_cursor_date_to_interval(cursor_time);
            } else {
                TimeInterval final_interval;
                return final_interval;
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
    auto date = std::chrono::floor<std::chrono::days>(cursor_time);
    auto temp_internal_interval_date = std::chrono::floor<std::chrono::days>(interval.start);

    auto time_of_day_start = interval.start - temp_internal_interval_date;
    auto time_of_day_end = interval.end - temp_internal_interval_date;

    // Відкидання секунд, мілісекунд і так далі
    time_of_day_start = std::chrono::floor<std::chrono::minutes>(time_of_day_start);
    time_of_day_end = std::chrono::floor<std::chrono::minutes>(time_of_day_end);
    
    auto final_start = date + time_of_day_start;
    auto final_end = date + time_of_day_end;

    TimeInterval final_interval{final_start, final_end};
    return final_interval;
}


