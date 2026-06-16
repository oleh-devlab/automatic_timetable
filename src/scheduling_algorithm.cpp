#include "scheduling_algorithm.h"

#include "task.h"
#include "time_blocks.h"

#include <vector>
#include <chrono>
#include <algorithm>

using namespace std::chrono_literals;

// --- Internal helpers ----------------------------------------------------

namespace {

bool is_zero_interval(const TimeInterval& iv) {
    return iv.end == PointInTime{};
}

// -- Expand repeatable TimeBlocks into concrete intervals -----------------
// One-time blocks will produce the same interval each
// day, but the subsequent merge step eliminates duplicates.
std::vector<TimeInterval> expand_time_blocks(
    std::vector<TimeBlock>& blocks,
    PointInTime period_begin,
    PointInTime period_end)
{
    std::vector<TimeInterval> occupied;

    auto start_day = std::chrono::floor<std::chrono::days>(period_begin);
    auto end_day   = std::chrono::floor<std::chrono::days>(period_end) + std::chrono::days{1};

    for (auto day = start_day; day <= end_day; day += std::chrono::days{1}) {
        PointInTime day_point{day};
        for (auto& block : blocks) {
            TimeInterval iv = block.get_interval(day_point);
            if (!is_zero_interval(iv) && iv.end > period_begin) {
                occupied.push_back(iv);
            }
        }
    }
    return occupied;
}

// Sort by start time, then merge overlapping / adjacent intervals.
std::vector<TimeInterval> merge_intervals(std::vector<TimeInterval> intervals) {
    if (intervals.empty()) return {};

    std::sort(intervals.begin(), intervals.end(),
        [](const TimeInterval& a, const TimeInterval& b) {
            return a.start < b.start;
        });

    std::vector<TimeInterval> merged;
    merged.push_back(intervals[0]);

    for (size_t i = 1; i < intervals.size(); ++i) {
        if (intervals[i].start <= merged.back().end) {
            if (intervals[i].end > merged.back().end)
                merged.back().end = intervals[i].end;
        } else {
            merged.push_back(intervals[i]);
        }
    }
    return merged;
}

// -- Per-task scheduling state --------------------------------------------
// Tracks how many sessions of a Task remain to be scheduled.
struct TaskProgress {
    int next_session = 1; // 1-based: next session to schedule
    int total_sessions = 0;
    std::chrono::minutes remaining{0};

    bool is_completed() const { return next_session > total_sessions; }

    std::chrono::minutes next_session_duration(const Task& task) const {
        return std::min(remaining, task.work_session_duration);
    }
};

} // anonymous namespace

// --- Public API ----------------------------------------------------------

std::vector<FinalBlock> get_schedule(std::vector<TimeBlock> time_blocks, std::vector<Task> tasks) {
    // -- Step 2.1: Initialise the cursor with the current local time ------
    PointInTime cursor = std::chrono::floor<std::chrono::minutes>(std::chrono::system_clock::now());
    std::vector<FinalBlock> schedule;

    if (tasks.empty()) return schedule;

    // -- Step 1: Sort tasks -----------------------------------------------
    // Primary key - deadline rounded to the nearest day (ascending).
    // Secondary key - priority (descending, i.e. higher = more important).
    // Tasks without a deadline go to the very end.
    std::sort(tasks.begin(), tasks.end(),
        [](const Task& a, const Task& b) {
            const bool a_has = a.deadline.has_value();
            const bool b_has = b.deadline.has_value();

            if (a_has && b_has) {
                auto day_a = std::chrono::floor<std::chrono::days>(*a.deadline);
                auto day_b = std::chrono::floor<std::chrono::days>(*b.deadline);
                if (day_a != day_b) return day_a < day_b;
                return a.priority > b.priority;       // same day -> higher priority first
            }
            if (a_has) return true;   // a has deadline, b doesn't -> a first
            if (b_has) return false;  // b has deadline, a doesn't -> b first
            return a.priority > b.priority;            // neither - sort by priority
        });

    // -- Initialise per-task progress -------------------------------------
    std::vector<TaskProgress> progress(tasks.size());
    size_t active_count = 0;

    for (size_t i = 0; i < tasks.size(); ++i) {
        const auto& t = tasks[i];
        if (t.total_duration.count() <= 0) continue;

        int ns = static_cast<int>(
            (t.total_duration.count()
                + t.work_session_duration.count() - 1)
            / t.work_session_duration.count());

        progress[i] = {1, ns, t.total_duration};
        ++active_count;
    }

    if (active_count == 0) return schedule;

    // -- Sliding-window: generate occupied blocks in 7-day chunks ---------
    const auto CHUNK_SIZE = std::chrono::days{7};

    PointInTime chunk_start = cursor;
    PointInTime chunk_end = chunk_start + CHUNK_SIZE;

    auto merged = merge_intervals(
                      expand_time_blocks(time_blocks, chunk_start, chunk_end));
    size_t block_idx = 0;

    // Safety limit: generous enough for many weeks of scheduling.
    int safety = static_cast<int>(active_count) * 10000 + 100000;

    // -- Steps 2.2 – 2.5: Main scheduling loop ----------------------------

    while (active_count > 0 && safety-- > 0) {
        if (cursor >= chunk_end || block_idx >= merged.size()) {
            chunk_start = chunk_end;
            chunk_end = chunk_start + CHUNK_SIZE;

            merged = merge_intervals(
                         expand_time_blocks(time_blocks, chunk_start, chunk_end));
            block_idx = 0;

            // If cursor lagged behind the new window start, pull it forward.
            if (cursor < chunk_start)
                cursor = chunk_start;

            continue;
        }

        // -- Step 2.2: Find the nearest occupied block from the future ----
        while (block_idx < merged.size() && merged[block_idx].end <= cursor)
            ++block_idx;

        // If we've exhausted blocks in this chunk, loop back to advance.
        if (block_idx >= merged.size())
            continue;

        PointInTime free_end = chunk_end;

        if (merged[block_idx].start > cursor) {
            free_end = merged[block_idx].start;
        } else {
            // Cursor sits inside an occupied block - skip past it.
            cursor = merged[block_idx].end + 1min;
            ++block_idx;
            continue;
        }

        auto free_minutes = std::chrono::duration_cast<std::chrono::minutes>(
                                free_end - cursor);
        if (free_minutes <= 0min) {
            cursor = merged[block_idx].end + 1min;
            ++block_idx;
            continue;
        }

        // -- Step 2.3 / 2.4: Fill the free window with sessions -----------
        bool scheduled_any = false;

        for (size_t i = 0; i < tasks.size(); ++i) {
            if (progress[i].is_completed()) continue;

            const Task& task = tasks[i];
            TaskProgress& prog = progress[i];

            // Try to schedule consecutive sessions of this task.
            while (!prog.is_completed()) {
                free_minutes = std::chrono::duration_cast<std::chrono::minutes>(
                                   free_end - cursor);
                if (free_minutes <= 0min) break;

                auto needed = prog.next_session_duration(task);

                if (needed > free_minutes)
                    break;   // doesn't fit -> try next task

                PointInTime session_end = cursor + needed;

                if (task.deadline.has_value() && session_end > *task.deadline) {

                    // -- Exception handling -------------------------------
                    if (task.min_session_duration.has_value())
                    {
                        auto min_dur = *task.min_session_duration;
                        auto avail   = std::chrono::duration_cast<std::chrono::minutes>(
                                           *task.deadline - cursor);

                        if (avail >= min_dur && min_dur <= free_minutes) {
                            auto shrunk = std::min(free_minutes, avail);
                            session_end = cursor + shrunk;

                            schedule.emplace_back(
                                task,
                                TimeInterval{cursor, session_end},
                                prog.next_session, prog.total_sessions,
                                "Скорочено сесію через дедлайн");

                            prog.remaining -= shrunk;
                            prog.next_session++;
                            if (prog.is_completed()) --active_count;

                            cursor = session_end;
                            scheduled_any = true;

                            // Insert a break if it fits.
                            auto rem = std::chrono::duration_cast<std::chrono::minutes>(
                                           free_end - cursor);
                            if (task.break_duration > 0min
                                && task.break_duration <= rem)
                            {
                                auto brk_end = cursor + task.break_duration;
                                schedule.emplace_back(
                                    TimeInterval{cursor, brk_end}, "Перерва");
                                cursor = brk_end;
                            }
                            continue; // try next session of same task
                        }
                    }

                    // Cannot fit - drop the remaining sessions (step 2.4).
                    schedule.emplace_back(
                        task,
                        TimeInterval{cursor, cursor},
                        prog.next_session, prog.total_sessions,
                        "Неможливо вмістити - дедлайн перевищено");

                    prog.next_session = prog.total_sessions + 1;
                    --active_count;
                    break; // move to next task
                }

                // -- Schedule the session ---------------------------------
                schedule.emplace_back(
                    task,
                    TimeInterval{cursor, session_end},
                    prog.next_session, prog.total_sessions);

                prog.remaining -= needed;
                prog.next_session++;
                if (prog.is_completed()) --active_count;

                cursor = session_end;
                scheduled_any = true;

                // -- Insert a break (Pomodoro rest) -----------------------
                auto break_dur     = task.break_duration;
                auto remaining_gap = std::chrono::duration_cast<std::chrono::minutes>(
                                         free_end - cursor);

                if (break_dur > 0min) {
                    if (break_dur <= remaining_gap) {
                        auto brk_end = cursor + break_dur;
                        schedule.emplace_back(
                            TimeInterval{cursor, brk_end}, "Перерва");
                        cursor = brk_end;

                    } else if (block_idx < merged.size()) {
                        auto block_dur = std::chrono::duration_cast<std::chrono::minutes>(
                            merged[block_idx].end - merged[block_idx].start);

                        if (block_dur >= (break_dur - 5min)) {
                            // The block acts as a break - nothing to insert.
                        } else if (remaining_gap > 0min) {
                            schedule.emplace_back(
                                TimeInterval{cursor, free_end},
                                "Скорочена перерва (до зайнятого блоку)");
                            cursor = free_end;
                        }
                    }
                }
            } // while (sessions of task i)

            // If the free window is exhausted, stop trying more tasks.
            free_minutes = std::chrono::duration_cast<std::chrono::minutes>(
                               free_end - cursor);
            if (free_minutes <= 0min) break;

        } // for (tasks)

        // -- Step 2.5: Advance the cursor ---------------------------------
        if (!scheduled_any) {
            if (block_idx < merged.size()) {
                cursor = merged[block_idx].end + 1min;
                ++block_idx;
            } else {
                // All blocks in this chunk exhausted - next iteration will trigger a window advance at the top of the loop.
                continue;
            }
        }
    }

    if (!schedule.empty() && !schedule.back().is_task) {
        schedule.pop_back();
    }

    return schedule;
}