#include "memory/MemoryStore.h"
#include <fstream>
#include <algorithm>
namespace core::memory { MemoryStore::MemoryStore(std::string p):p_(std::move(p)){std::ifstream f(p_);Memory m;while(f>>m.id>>m.scope){f.ignore(1);std::getline(f,m.text);next_=std::max(next_,m.id+1);}} bool MemoryStore::add(std::string scope,std::string text){std::ofstream f(p_,std::ios::app);if(!f)return false;f<<next_++<<' '<<scope<<' '<<text<<'\n';return true;} std::vector<Memory> MemoryStore::search(const std::string&q,size_t n)const{std::vector<Memory>o;std::ifstream f(p_);Memory m;while(f>>m.id>>m.scope){f.ignore(1);std::getline(f,m.text);if(q.empty()||m.text.find(q)!=std::string::npos||m.scope.find(q)!=std::string::npos)o.push_back(m);if(o.size()>=n)break;}return o;} }
