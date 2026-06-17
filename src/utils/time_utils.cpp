#include "time_utils.h"

#include <ctime>
#include <cstdio>

PointInTime make_time_point(int year, int month, int day, int hours, int minutes)
{
    std::tm tm{};
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hours;
    tm.tm_min = minutes;
    tm.tm_sec = 0;
    tm.tm_isdst = -1;
    std::time_t t = std::mktime(&tm);
    return std::chrono::floor<std::chrono::minutes>(std::chrono::system_clock::from_time_t(t));
}

PointInTime make_time_point_today(int days_offset, int hours, int minutes)
{
    std::time_t now_t = std::time(nullptr);
    std::tm tm = *std::localtime(&now_t);
    tm.tm_mday += days_offset;
    tm.tm_hour = hours;
    tm.tm_min = minutes;
    tm.tm_sec = 0;
    tm.tm_isdst = -1;
    return std::chrono::floor<std::chrono::minutes>(std::chrono::system_clock::from_time_t(std::mktime(&tm)));
}

std::string fmt_hhmm(PointInTime tp)
{
    std::time_t t = std::chrono::system_clock::to_time_t(tp);
    std::tm tm = *std::localtime(&t);
    char buf[8];
    std::snprintf(buf, sizeof(buf), "%02d:%02d", tm.tm_hour, tm.tm_min);
    return buf;
}

std::string fmt_datetime(PointInTime tp) {
    if (tp == PointInTime{}) return "--:--";

    std::time_t t = std::chrono::system_clock::to_time_t(tp);
    std::tm tm = *std::localtime(&t);

    char buf[32];
    std::snprintf(buf, sizeof(buf), "%02u.%02u.%04d %02d:%02d",
                  static_cast<unsigned>(tm.tm_mday),
                  static_cast<unsigned>(tm.tm_mon + 1),
                  static_cast<int>(tm.tm_year + 1900),
                  static_cast<int>(tm.tm_hour),
                  static_cast<int>(tm.tm_min));
    return buf;
}

const char* weekday_name(std::chrono::weekday wd)
{
    static const char* names[] = {
        "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
    };
    return names[wd.c_encoding()];
}
