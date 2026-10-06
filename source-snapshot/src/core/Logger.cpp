#include "core/Logger.h"
#include <fstream>
#include <chrono>
namespace coreai {
Logger::Logger(std::filesystem::path path):path_(std::move(path)){}
void Logger::write(const char* level,const std::string& m){std::lock_guard<std::mutex> g(mutex_); std::ofstream f(path_,std::ios::app); if(f){auto t=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()); f<<t<<" ["<<level<<"] "<<m<<'\n';}}
void Logger::debug(const std::string&m){write("DEBUG",m);} void Logger::info(const std::string&m){write("INFO",m);} void Logger::warn(const std::string&m){write("WARN",m);} void Logger::error(const std::string&m){write("ERROR",m);} void Logger::fatal(const std::string&m){write("FATAL",m);}
}
