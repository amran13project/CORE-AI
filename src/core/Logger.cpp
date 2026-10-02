#include "core/Logger.h"
#include <fstream>
#include <chrono>
namespace core { static long long t(){return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();} Logger::Logger(std::string p):p_(std::move(p)){} void Logger::info(const std::string&s){std::ofstream f(p_,std::ios::app);if(f)f<<t()<<" INFO "<<s<<'\n';} void Logger::error(const std::string&s){std::ofstream f(p_,std::ios::app);if(f)f<<t()<<" ERROR "<<s<<'\n';} }
