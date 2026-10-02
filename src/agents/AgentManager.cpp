#include "agents/AgentManager.h"
#include <sstream>
namespace core::agents { std::string AgentManager::planOnly(const std::string&r)const{std::ostringstream o;for(auto&s:p_.plan(r))o<<s.index<<". "<<s.action<<'\n';return o.str();} }
