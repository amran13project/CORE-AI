#pragma once
#include <string>
#include <vector>
namespace core::agents {
struct PlanStep { int index=0; std::string action; std::string detail; };
class Planner {
public:
    std::vector<PlanStep> create(const std::string& request) const;
};
}
