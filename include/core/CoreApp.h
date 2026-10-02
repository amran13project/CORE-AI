#pragma once
#include "core/Config.h"
#include "core/Logger.h"
#include "core/EventBus.h"
#include "ai/ModelRouter.h"
#include "audit/AuditLog.h"
#include "automation/WorkflowEngine.h"
#include "memory/MemoryStore.h"
#include "projects/ProjectManager.h"
#include "security/PermissionManager.h"
#include "tools/ToolRegistry.h"
#include "agents/AgentManager.h"
#include <string>
namespace core { class CoreApp { std::string root_; Config config_; Logger logger_; EventBus events_; ai::ModelRouter models_; memory::MemoryStore memory_; tools::ToolRegistry tools_; security::PermissionManager permissions_; audit::AuditLog audit_; projects::ProjectManager projects_; agents::AgentManager agents_; automation::WorkflowEngine workflows_; public: CoreApp(); bool initialize(const std::string&); std::string chat(const std::string&); std::string think(const std::string&); std::string code(const std::string&); std::string agent(const std::string&); std::string capabilities() const; std::string status() const; std::string memorySearch(const std::string&) const; bool remember(const std::string&,const std::string&); std::string projectCreate(const std::string&); std::string projectList() const; void grant(const std::string&); bool hasPermission(const std::string&) const; std::string webSearchUrl(const std::string&) const; std::string mapUrl(const std::string&) const; std::string directionsUrl(const std::string&,const std::string&) const; };
}
