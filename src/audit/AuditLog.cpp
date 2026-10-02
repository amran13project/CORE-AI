#include "audit/AuditLog.h"
#include <fstream>
#include <chrono>
namespace core::audit { AuditLog::AuditLog(std::string p):p_(std::move(p)){} void AuditLog::record(const std::string&a,const std::string&x,const std::string&r)const{std::ofstream f(p_,std::ios::app);if(f)f<<std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count()<<'\t'<<a<<'\t'<<x<<'\t'<<r<<'\n';} }
