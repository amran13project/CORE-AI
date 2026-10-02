#pragma once
#include <fstream>
#include <mutex>
#include <string>

namespace core {
class Logger {
public:
    explicit Logger(const std::string& file = "core.log");
    void info(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);
private:
    void write(const char* level, const std::string& message);
    std::ofstream file_;
    std::mutex mutex_;
};
}
