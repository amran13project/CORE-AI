#pragma once
#include <string>
#include <vector>
namespace core::agents { struct Step{int index;std::string action;}; class Planner{public:std::vector<Step> plan(const std::string&)const;}; }
