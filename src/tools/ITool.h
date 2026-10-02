#pragma once
#include <string>
namespace core::tools {
struct ToolContext { std::string workingDirectory; };
struct ToolResult { bool ok=false; std::string output; int exitCode=-1; };
class ITool {
public:
    virtual ~ITool() = default;
    virtual const char* id() const = 0;
    virtual const char* description() const = 0;
    virtual ToolResult invoke(const std::string& input, const ToolContext& context) = 0;
};
}
