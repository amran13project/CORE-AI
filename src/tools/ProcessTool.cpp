#include "tools/ProcessTool.h"
#include <cstdio>
#include <array>
#include <filesystem>
#if defined(_WIN32)
#define CORE_POPEN _popen
#define CORE_PCLOSE _pclose
#else
#define CORE_POPEN popen
#define CORE_PCLOSE pclose
#endif
namespace core::tools {
ToolResult ProcessTool::invoke(const std::string& input, const ToolContext& context) {
    const std::string command = "cd /d \"" + context.workingDirectory + "\" 2>NUL & " + input + " 2>&1";
#if !defined(_WIN32)
    const std::string unixCommand = "cd '" + context.workingDirectory + "' && " + input + " 2>&1";
    FILE* pipe = CORE_POPEN(unixCommand.c_str(), "r");
#else
    FILE* pipe = CORE_POPEN(command.c_str(), "r");
#endif
    if (!pipe) return {false,"Unable to start process",-1};
    std::array<char,4096> buffer{}; std::string output;
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe)) output += buffer.data();
    const int rc = CORE_PCLOSE(pipe);
    return {rc == 0, output, rc};
}
}
