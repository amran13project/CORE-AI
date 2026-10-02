#pragma once
#include <string>
#include <unordered_set>
namespace core::security { class PermissionManager { std::unordered_set<std::string>a_; public: PermissionManager(); bool allows(const std::string&)const; void grant(const std::string&); void revoke(const std::string&); }; }
