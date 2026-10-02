#pragma once
#include "tools/ITool.h"
namespace core::tools {
class FileTool final : public ITool {
public:
    const char* id() const override { return "file"; }
    const char* description() const override { return "Read or write UTF-8 text files."; }
    ToolResult invoke(const std::string& input, const ToolContext& context) override;
};
}
