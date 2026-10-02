#pragma once
#include "tools/ITool.h"
namespace core::tools {
class ProcessTool final : public ITool {
public:
    const char* id() const override { return "process"; }
    const char* description() const override { return "Run a shell command in a selected working directory."; }
    ToolResult invoke(const std::string& input, const ToolContext& context) override;
};
}
