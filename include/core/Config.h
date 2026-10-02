#pragma once
#include <string>
#include <unordered_map>
namespace core { class Config { std::unordered_map<std::string,std::string> v_; public: bool load(const std::string&); bool save(const std::string&) const; void set(std::string,std::string); std::string get(const std::string&,const std::string& = {}) const; }; }
