#include "ai/ModelRouter.h"
namespace core::ai { void ModelRouter::add(std::unique_ptr<IAIProvider>p){p_.push_back(std::move(p));} IAIProvider* ModelRouter::select()const{for(auto&i:p_)if(i->available())return i.get();return p_.empty()?nullptr:p_.front().get();} std::vector<std::string> ModelRouter::ids()const{std::vector<std::string>o;for(auto&i:p_)o.push_back(i->id());return o;} }
