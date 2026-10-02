#pragma once
#include <string>
#include <unordered_map>
#include <vector>
namespace core::automation { class WorkflowEngine { std::unordered_map<std::string,std::vector<std::string>> w_; public: void set(std::string,std::vector<std::string>); std::vector<std::string> actions(const std::string&)const; }; }
