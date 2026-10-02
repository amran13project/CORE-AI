#include "automation/WorkflowEngine.h"
namespace core::automation {
void WorkflowEngine::add(Workflow workflow){workflows_.push_back(std::move(workflow));}
const Workflow* WorkflowEngine::find(const std::string& id) const{for(const auto& w:workflows_)if(w.id==id)return &w;return nullptr;}
std::vector<std::string> WorkflowEngine::ids() const{std::vector<std::string>r;for(const auto&w:workflows_)r.push_back(w.id);return r;}
}
