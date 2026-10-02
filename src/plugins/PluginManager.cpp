#include "plugins/PluginManager.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
namespace core::plugins { bool PluginManager::load(const std::string&p){
#ifdef _WIN32
HMODULE h=LoadLibraryA(p.c_str());if(!h)return false;
#else
void*h=dlopen(p.c_str(),RTLD_NOW);if(!h)return false;
#endif
loaded_.push_back(p);return true; } }
