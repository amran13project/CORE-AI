#include "security/PermissionManager.h"
namespace core::security {
PermissionManager::PermissionManager() { allowed_.insert("memory.read"); allowed_.insert("memory.write"); allowed_.insert("file.read"); }
bool PermissionManager::allows(const std::string& permission) const { return allowed_.contains(permission); }
void PermissionManager::grant(const std::string& permission) { allowed_.insert(permission); }
void PermissionManager::revoke(const std::string& permission) { allowed_.erase(permission); }
}
