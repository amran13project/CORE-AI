#include "security/PermissionManager.h"
namespace core::security { PermissionManager::PermissionManager(){a_={"memory.read","memory.write","file.read"};} bool PermissionManager::allows(const std::string&s)const{return a_.contains(s);} void PermissionManager::grant(const std::string&s){a_.insert(s);} void PermissionManager::revoke(const std::string&s){a_.erase(s);} }
