#include "tools/ToolRegistry.h"
namespace core::tools { void registerProcessTool(ToolRegistry&r){r.add("process.run",[](const std::string&c){return std::string("PROCESS DISABLED BY DEFAULT: ")+c;});} }
