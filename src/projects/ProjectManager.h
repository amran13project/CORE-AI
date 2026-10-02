#pragma once
#include <string>
namespace core::projects {
struct Project { std::string name; std::string root; };
class ProjectManager {
public:
    Project open(const std::string& root) const;
    bool create(const std::string& root, const std::string& name) const;
};
}
