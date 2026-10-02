#pragma once
#include <string>
#include <unordered_set>
namespace core::security {
class PermissionManager {
public:
    PermissionManager();
    bool allows(const std::string& permission) const;
    void grant(const std::string& permission);
    void revoke(const std::string& permission);
private:
    std::unordered_set<std::string> allowed_;
};
}
