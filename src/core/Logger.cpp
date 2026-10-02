#include "core/Logger.h"
#include <chrono>
#include <ctime>
#include <iostream>

namespace core {
Logger::Logger(const std::string& file) : file_(file, std::ios::app) {}
void Logger::write(const char* level, const std::string& message) {
    std::lock_guard lock(mutex_);
    const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char stamp[32]{};
#if defined(_WIN32)
    std::tm tm{}; localtime_s(&tm, &now); std::strftime(stamp, sizeof(stamp), "%Y-%m-%d %H:%M:%S", &tm);
#else
    std::tm tm{}; localtime_r(&now, &tm); std::strftime(stamp, sizeof(stamp), "%Y-%m-%d %H:%M:%S", &tm);
#endif
    const std::string line = std::string("[") + stamp + "][" + level + "] " + message;
    std::cout << line << '\n';
    if (file_) file_ << line << '\n';
}
void Logger::info(const std::string& message) { write("INFO", message); }
void Logger::warn(const std::string& message) { write("WARN", message); }
void Logger::error(const std::string& message) { write("ERROR", message); }
}
