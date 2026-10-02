#include "audit/AuditLog.h"
#include <chrono>
namespace core::audit {
AuditLog::AuditLog(std::string path):out_(std::move(path),std::ios::app){}
void AuditLog::record(const std::string& actor,const std::string& action,const std::string& result){ if(out_) out_<<std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())<<'\t'<<actor<<'\t'<<action<<'\t'<<result<<'\n'; }
}
