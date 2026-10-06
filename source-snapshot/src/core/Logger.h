#pragma once
#include <filesystem>
#include <mutex>
#include <string>
namespace coreai {
class Logger {
public:
    explicit Logger(std::filesystem::path path);
    void debug(const std::string& m); void info(const std::string& m); void warn(const std::string& m); void error(const std::string& m); void fatal(const std::string& m);
private:
    void write(const char* level, const std::string& m);
    std::filesystem::path path_; std::mutex mutex_;
};
}
