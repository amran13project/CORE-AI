#pragma once
#include "agents/Planner.h"
#include "ai/ModelRouter.h"
#include "tools/ToolRegistry.h"
#include "security/PermissionManager.h"
#include "audit/AuditLog.h"
#include <string>
namespace core::agents {
class AgentManager {
public:
    AgentManager(ai::ModelRouter& models, tools::ToolRegistry& tools, security::PermissionManager& permissions, audit::AuditLog& audit);
    std::string run(const std::string& request, const std::string& workingDirectory);
private:
    ai::ModelRouter& models_;
    tools::ToolRegistry& tools_;
    security::PermissionManager& permissions_;
    audit::AuditLog& audit_;
    Planner planner_;
};
}
