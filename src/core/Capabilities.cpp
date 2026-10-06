#include "core/Capabilities.h"
#include <sstream>
namespace coreai { static std::string esc(const std::string&s){std::string o;for(char c:s){if(c=='"')o+="\\\"";else if(c=='\\')o+="\\\\";else if(c=='\n')o+="\\n";else o+=c;}return o;}
void CapabilityRegistry::set(Capability c){for(auto&i:items_)if(i.id==c.id){i=std::move(c);return;}items_.push_back(std::move(c));}
std::vector<Capability> CapabilityRegistry::all()const{return items_;}
std::string CapabilityRegistry::json()const{std::ostringstream o;o<<"[";for(size_t i=0;i<items_.size();++i){auto&c=items_[i];if(i)o<<',';o<<"{\"id\":\""<<esc(c.id)<<"\",\"name\":\""<<esc(c.name)<<"\",\"status\":\""<<c.status<<"\",\"reason\":\""<<esc(c.reason)<<"\",\"implemented\":"<<(c.implemented?"true":"false")<<",\"available\":"<<(c.available?"true":"false")<<",\"configured\":"<<(c.configured?"true":"false")<<",\"enabled\":"<<(c.enabled?"true":"false")<<"}";}o<<"]";return o.str();}
}
