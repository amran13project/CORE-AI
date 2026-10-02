#pragma once
#include "tools/ITool.h"
#include <memory>
#include <unordered_map>
#include <vector>
namespace core::tools {
class ToolRegistry {
public:
    void add(std::unique_ptr<ITool> tool);
    ITool* find(const std::string& id) const;
    ToolResult invoke(const std::string& id, const std::string& input, const ToolContext& context) const;
    std::vector<std::string> ids() const;
private:
    std::unordered_map<std::string, std::unique_ptr<ITool>> tools_;
};
}
