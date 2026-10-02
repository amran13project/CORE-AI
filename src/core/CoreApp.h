#pragma once
#include "core/Config.h"
#include "core/Logger.h"
#include "core/EventBus.h"
#include "ai/ModelRouter.h"
#include "memory/MemoryStore.h"
#include "tools/ToolRegistry.h"
#include "security/PermissionManager.h"
#include "audit/AuditLog.h"
#include "agents/AgentManager.h"
#include "verification/VerificationEngine.h"
#include "plugins/PluginManager.h"
#include "projects/ProjectManager.h"
#include "automation/WorkflowEngine.h"
#include <string>
namespace core {
class CoreApp : public plugins::CoreAPI {
public:
    CoreApp();
    bool initialize(const std::string& root = ".");
    std::string chat(const std::string& prompt);
    void addMemory(const std::string& scope, const std::string& text);
    std::string searchMemory(const std::string& query) const;
    std::string runAgent(const std::string& request);
    std::string tool(const std::string& id,const std::string& input,const std::string& cwd);
    std::string doctor() const;
    std::string capabilities() const;
    void log(const std::string& message) override { logger_.info(message); }
private:
    std::string root_;
    Config config_;
    Logger logger_;
    EventBus events_;
    ai::ModelRouter models_;
    memory::MemoryStore memory_;
    tools::ToolRegistry tools_;
    security::PermissionManager permissions_;
    audit::AuditLog audit_;
    verification::VerificationEngine verification_;
    projects::ProjectManager projects_;
    automation::WorkflowEngine workflows_;
    plugins::PluginManager plugins_;
    std::unique_ptr<agents::AgentManager> agents_;
};
}
