#include "tools/ToolRegistry.h"
#include <algorithm>
namespace core::tools { void ToolRegistry::add(std::string n,Tool f){t_[std::move(n)]=std::move(f);} std::string ToolRegistry::invoke(const std::string&n,const std::string&i)const{auto x=t_.find(n);return x==t_.end()?"UNKNOWN TOOL":x->second(i);} std::vector<std::string> ToolRegistry::ids()const{std::vector<std::string>o;for(auto&x:t_)o.push_back(x.first);std::sort(o.begin(),o.end());return o;} }
