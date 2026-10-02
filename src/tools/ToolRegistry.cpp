#include "tools/ToolRegistry.h"
namespace core::tools {
void ToolRegistry::add(std::unique_ptr<ITool> tool) { tools_[tool->id()] = std::move(tool); }
ITool* ToolRegistry::find(const std::string& id) const { auto it=tools_.find(id); return it==tools_.end()?nullptr:it->second.get(); }
ToolResult ToolRegistry::invoke(const std::string& id, const std::string& input, const ToolContext& context) const { auto* t=find(id); return t?t->invoke(input,context):ToolResult{false,"Unknown tool: "+id,-1}; }
std::vector<std::string> ToolRegistry::ids() const { std::vector<std::string> r; for (const auto& [id,_]:tools_) r.push_back(id); return r; }
}
