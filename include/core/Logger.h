#pragma once
#include <string>
namespace core { class Logger { std::string p_; public: explicit Logger(std::string); void info(const std::string&); void error(const std::string&); }; }
