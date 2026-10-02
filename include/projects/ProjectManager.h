#pragma once
#include <string>
#include <vector>
namespace core::projects { class ProjectManager { std::string root_; public: explicit ProjectManager(std::string); bool create(const std::string&); std::vector<std::string> list()const; }; }
