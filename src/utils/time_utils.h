#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <chrono>
#include <string>

#include "../core/time_blocks.h"

PointInTime make_time_point(int year, int month, int day, int hours, int minutes);
PointInTime make_time_point_today(int days_offset, int hours, int minutes);
std::string fmt_hhmm(PointInTime tp);
std::string fmt_datetime(PointInTime tp);
const char* weekday_name(std::chrono::weekday wd);

#endif
