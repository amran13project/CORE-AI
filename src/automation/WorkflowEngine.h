#pragma once
#include <string>
#include <vector>
namespace core::automation {
struct Action { std::string type; std::string payload; };
struct Workflow { std::string id; std::string trigger; std::vector<Action> actions; };
class WorkflowEngine {
public:
    void add(Workflow workflow);
    const Workflow* find(const std::string& id) const;
    std::vector<std::string> ids() const;
private:
    std::vector<Workflow> workflows_;
};
}
