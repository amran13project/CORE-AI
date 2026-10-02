#pragma once
#include <string>
#include <fstream>
namespace core::audit {
class AuditLog {
public:
    explicit AuditLog(std::string path = "audit.log");
    void record(const std::string& actor, const std::string& action, const std::string& result);
private:
    std::ofstream out_;
};
}
