#pragma once
#include <string>
namespace core::audit { class AuditLog { std::string p_; public: explicit AuditLog(std::string); void record(const std::string&,const std::string&,const std::string&) const; }; }
