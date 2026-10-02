#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
namespace core { class EventBus { using Fn=std::function<void(const std::string&)>; std::unordered_map<std::string,std::vector<Fn>> s_; public: void on(const std::string&,Fn); void emit(const std::string&,const std::string&) const; }; }
