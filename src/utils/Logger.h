#pragma once
#include <vector>
#include <string>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace KuroUtils {

    inline std::vector<std::string> debug_logs;
    inline std::mutex debug_log_mutex;
    inline bool show_debug_window = false;

    inline void Log(const std::string& msg) {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "[%H:%M:%S] ") << msg;
        
        std::lock_guard<std::mutex> lock(debug_log_mutex);
        debug_logs.push_back(ss.str());
        
        if (debug_logs.size() > 500) {
            debug_logs.erase(debug_logs.begin());
        }
    }
}
