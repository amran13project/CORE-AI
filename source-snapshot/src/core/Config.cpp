#include "core/Config.h"
#include <fstream>
#include <cctype>
namespace coreai {
static std::string trim(std::string s){auto ws=[](unsigned char c){return std::isspace(c);};while(!s.empty()&&ws((unsigned char)s.front()))s.erase(s.begin());while(!s.empty()&&ws((unsigned char)s.back()))s.pop_back();return s;}
Result<void> Config::load(const std::filesystem::path& p){std::ifstream f(p);if(!f)return Result<void>::failure(error(ErrorCode::ConfigInvalid,"config file unavailable","config","load",true));std::string l;while(std::getline(f,l)){l=trim(l);if(l.empty()||l[0]=='#')continue;auto x=l.find('=');if(x==std::string::npos)return Result<void>::failure(error(ErrorCode::ConfigInvalid,"invalid config line","config","parse"));auto k=trim(l.substr(0,x));auto v=trim(l.substr(x+1));if(k.empty())return Result<void>::failure(error(ErrorCode::ConfigInvalid,"empty config key","config","parse"));values_[k]=v;}return Result<void>::success();}
Result<void> Config::save(const std::filesystem::path&p)const{std::ofstream f(p,std::ios::trunc);if(!f)return Result<void>::failure(error(ErrorCode::ConfigInvalid,"cannot write config","config","save",true));for(auto&[k,v]:values_)f<<k<<'='<<v<<'\n';return Result<void>::success();}
void Config::set(std::string k,std::string v){values_[std::move(k)]=std::move(v);} std::string Config::get(const std::string&k,const std::string&d)const{auto i=values_.find(k);return i==values_.end()?d:i->second;} bool Config::boolean(const std::string&k,bool d)const{auto v=get(k,d?"true":"false");return v=="true"||v=="1"||v=="yes"||v=="on";}
}
