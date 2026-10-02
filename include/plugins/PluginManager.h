#pragma once
#include <string>
#include <vector>
namespace core::plugins { class PluginManager { std::vector<std::string>loaded_; public: bool load(const std::string&); std::size_t count()const{return loaded_.size();} }; }
