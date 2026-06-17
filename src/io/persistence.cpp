#include "persistence.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace {

const char* TASKS_FILE = "tasks.tsv";
const char* TIME_BLOCKS_FILE = "time_blocks.tsv";
const char* COMPLETED_TASKS_FILE = "completed_tasks.tsv";

const char* TASKS_HEADER = "id\tname\tdescription\thas_deadline\tdeadline\tpriority\ttotal_dur\tsession_dur\tbreak_dur\thas_min_session\tmin_session\n";
const char* TIME_BLOCKS_HEADER = "is_repeatable\tis_every_day\tstart_time\tend_time\tday_of_week\n";

// Cleans string so it doesn't break TSV format.
std::string clean_string(std::string s) {
    for (char& c : s) {
        if (c == '\t' || c == '\n' || c == '\r') {
            c = ' ';
        }
    }
    if (s.empty()) {
        s = " "; // to ensure token extraction doesn't skip empty fields at the end
    }
    return s;
}

int64_t serialize_time(PointInTime tp) {
    return tp.time_since_epoch().count();
}

PointInTime deserialize_time(int64_t mins) {
    return PointInTime{std::chrono::minutes{mins}};
}

// Split line by tab
std::vector<std::string> split_tsv(const std::string& line) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;
    while (std::getline(ss, token, '\t')) {
        tokens.push_back(token);
    }
    // Handle case where line ends with a tab
    if (!line.empty() && line.back() == '\t') {
        tokens.push_back("");
    }
    return tokens;
}

// --- Helpers for Task --------------------------------------------------------

void write_task_tsv(std::ostream& out, const Task& t) {
    out << t.id << "\t"
        << clean_string(t.name) << "\t"
        << clean_string(t.description) << "\t"
        << (t.deadline.has_value() ? 1 : 0) << "\t"
        << (t.deadline.has_value() ? serialize_time(*t.deadline) : 0) << "\t"
        << t.priority << "\t"
        << t.total_duration.count() << "\t"
        << t.work_session_duration.count() << "\t"
        << t.break_duration.count() << "\t"
        << (t.min_session_duration.has_value() ? 1 : 0) << "\t"
        << (t.min_session_duration.has_value() ? t.min_session_duration->count() : 0) << "\n";
}

bool parse_task_tsv(const std::vector<std::string>& cols, Task& t) {
    if (cols.size() < 11) return false;
    
    try {
        t.id = std::stoull(cols[0]);
        
        t.name = cols[1];
        if (t.name == " ") t.name = "";
        
        t.description = cols[2];
        if (t.description == " ") t.description = "";
        
        bool has_deadline = std::stoi(cols[3]);
        if (has_deadline) {
            t.deadline = deserialize_time(std::stoll(cols[4]));
        }
        
        t.priority = std::stoi(cols[5]);
        t.total_duration = std::chrono::minutes{std::stoll(cols[6])};
        t.work_session_duration = std::chrono::minutes{std::stoll(cols[7])};
        t.break_duration = std::chrono::minutes{std::stoll(cols[8])};
        
        bool has_min_session = std::stoi(cols[9]);
        if (has_min_session) {
            t.min_session_duration = std::chrono::minutes{std::stoll(cols[10])};
        }
        return true;
    } catch (...) {
        return false;
    }
}

// --- Helpers for TimeBlock ---------------------------------------------------

void write_time_block_tsv(std::ostream& out, const TimeBlock& tb) {
    auto iv = tb.get_raw_interval();
    out << (tb.get_repeatable() ? 1 : 0) << "\t"
        << (tb.get_every_day() ? 1 : 0) << "\t"
        << serialize_time(iv.start) << "\t"
        << serialize_time(iv.end) << "\t"
        << tb.get_day_of_week().c_encoding() << "\n";
}

bool parse_time_block_tsv(const std::vector<std::string>& cols, TimeBlock& tb) {
    if (cols.size() < 5) return false;
    
    try {
        bool is_repeatable = std::stoi(cols[0]);
        bool is_every_day = std::stoi(cols[1]);
        TimeInterval iv{deserialize_time(std::stoll(cols[2])), deserialize_time(std::stoll(cols[3]))};
        unsigned day_of_week = std::stoul(cols[4]);
        
        tb = TimeBlock{is_repeatable, is_every_day, iv, std::chrono::weekday{day_of_week}};
        return true;
    } catch (...) {
        return false;
    }
}

} // anonymous namespace

// --- Public API --------------------------------------------------------------

bool save_data(const std::vector<TimeBlock>& tbs, const std::vector<Task>& tasks) {
    bool ok = true;

    // Save Tasks
    std::ofstream out_tasks(TASKS_FILE);
    if (!out_tasks) {
        std::cerr << "  Error: could not open " << TASKS_FILE << " for writing.\n";
        ok = false;
    } else {
        out_tasks << TASKS_HEADER;
        for (const auto& t : tasks) {
            write_task_tsv(out_tasks, t);
        }
    }

    // Save TimeBlocks
    std::ofstream out_blocks(TIME_BLOCKS_FILE);
    if (!out_blocks) {
        std::cerr << "  Error: could not open " << TIME_BLOCKS_FILE << " for writing.\n";
        ok = false;
    } else {
        out_blocks << TIME_BLOCKS_HEADER;
        for (const auto& tb : tbs) {
            write_time_block_tsv(out_blocks, tb);
        }
    }
    
    return ok;
}

bool load_data(std::vector<TimeBlock>& tbs, std::vector<Task>& tasks, uint64_t& next_task_id) {
    tbs.clear();
    tasks.clear();
    next_task_id = 1;
    bool ok = true;

    // Load Tasks
    std::ifstream in_tasks(TASKS_FILE);
    if (in_tasks) {
        std::string line;
        std::getline(in_tasks, line); // Skip header
        
        while (std::getline(in_tasks, line)) {
            if (line.empty()) continue;
            
            Task t{};
            if (parse_task_tsv(split_tsv(line), t)) {
                tasks.push_back(std::move(t));
                if (t.id >= next_task_id) {
                    next_task_id = t.id + 1;
                }
            } else {
                std::cerr << "  Warning: error reading Task line.\n";
            }
        }
    }
    
    // Load TimeBlocks
    std::ifstream in_blocks(TIME_BLOCKS_FILE);
    if (in_blocks) {
        std::string line;
        std::getline(in_blocks, line); // Skip header
        
        while (std::getline(in_blocks, line)) {
            if (line.empty()) continue;
            
            TimeBlock tb = TimeBlock{false, false, TimeInterval{}, std::chrono::weekday{0}};
            if (parse_time_block_tsv(split_tsv(line), tb)) {
                tbs.push_back(std::move(tb));
            } else {
                std::cerr << "  Warning: error reading TimeBlock line.\n";
            }
        }
    }
    
    return ok;
}

void append_completed_task(const Task& t) {
    std::ofstream out(COMPLETED_TASKS_FILE, std::ios::app);
    if (!out) {
        std::cerr << "  Error: could not open " << COMPLETED_TASKS_FILE << " for writing.\n";
        return;
    }
    
    // Write header if file is empty
    out.seekp(0, std::ios::end);
    if (out.tellp() == 0) {
        out << TASKS_HEADER;
    }

    write_task_tsv(out, t);
}

void write_schedule_tsv(std::ostream& out, const std::vector<FinalBlock>& schedule) {
    const char* SCHEDULE_HEADER = "task_id\ttask_name\tis_task\tstart_time\tend_time\tsession_index\ttotal_sessions\talgo_notes\n";
    out << SCHEDULE_HEADER;
    for (const auto& block : schedule) {
        out << block.task_id << "\t"
            << clean_string(block.task_name) << "\t"
            << (block.is_task ? 1 : 0) << "\t"
            << serialize_time(block.interval.start) << "\t"
            << serialize_time(block.interval.end) << "\t"
            << block.session_index << "\t"
            << block.total_sessions << "\t"
            << clean_string(block.algo_notes) << "\n";
    }
}