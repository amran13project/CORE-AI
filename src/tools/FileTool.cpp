#include "tools/FileTool.h"
#include <fstream>
#include <filesystem>
namespace core::tools {
ToolResult FileTool::invoke(const std::string& input, const ToolContext& context) {
    const auto pos = input.find('|'); if (pos == std::string::npos) return {false, "file tool format: read|path OR write|path|content", -1};
    const auto op = input.substr(0,pos); const auto rest = input.substr(pos+1);
    if (op == "read") {
        std::ifstream in(std::filesystem::path(context.workingDirectory) / rest);
        if (!in) return {false, "Cannot read file: " + rest, -1};
        return {true, std::string((std::istreambuf_iterator<char>(in)), {}), 0};
    }
    if (op == "write") {
        const auto p2 = rest.find('|'); if (p2 == std::string::npos) return {false, "write requires path|content", -1};
        const auto path = std::filesystem::path(context.workingDirectory) / rest.substr(0,p2);
        std::filesystem::create_directories(path.parent_path());
        std::ofstream out(path, std::ios::binary | std::ios::trunc); if (!out) return {false,"Cannot write file: "+path.string(),-1};
        out << rest.substr(p2+1); return {true,"Wrote: "+path.string(),0};
    }
    return {false, "Unknown file op: " + op, -1};
}
}
