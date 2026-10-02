#include "core/EventBus.h"
namespace core { void EventBus::on(const std::string&e,Fn f){s_[e].push_back(std::move(f));} void EventBus::emit(const std::string&e,const std::string&p)const{auto i=s_.find(e);if(i!=s_.end())for(auto&f:i->second)f(p);} }
