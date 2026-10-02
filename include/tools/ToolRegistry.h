#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
namespace core::tools { using Tool=std::function<std::string(const std::string&)>; class ToolRegistry { std::unordered_map<std::string,Tool>t_; public: void add(std::string,Tool); std::string invoke(const std::string&,const std::string&)const; std::vector<std::string> ids()const; }; }
