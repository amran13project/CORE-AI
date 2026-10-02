#include "automation/WorkflowEngine.h"
namespace core::automation { void WorkflowEngine::set(std::string e,std::vector<std::string>a){w_[std::move(e)]=std::move(a);} std::vector<std::string> WorkflowEngine::actions(const std::string&e)const{auto i=w_.find(e);return i==w_.end()?std::vector<std::string>{}:i->second;} }
