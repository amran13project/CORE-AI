#include "plugins/PluginManager.h"
#include <filesystem>
#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif
namespace core::plugins {
PluginManager::PluginManager(CoreAPI& api):api_(api){}
PluginManager::~PluginManager(){ for(auto& p:loaded_){if(p.plugin)p.plugin->shutdown();
#if defined(_WIN32)
 if(p.module) FreeLibrary(static_cast<HMODULE>(p.module));
#else
 if(p.module) dlclose(p.module);
#endif
 }}
size_t PluginManager::discover(const std::string& directory){
    if(!std::filesystem::exists(directory)) return 0;
    for(const auto& e:std::filesystem::directory_iterator(directory)){
        if(!e.is_regular_file()) continue;
#if defined(_WIN32)
        if(e.path().extension() != ".dll") continue;
        HMODULE mod=LoadLibraryW(e.path().wstring().c_str()); if(!mod) continue;
        auto create=reinterpret_cast<CreatePluginFn>(GetProcAddress(mod,"core_create_plugin")); if(!create){FreeLibrary(mod);continue;}
        auto* plugin=create(); if(!plugin){FreeLibrary(mod);continue;}
#else
        if(e.path().extension() != ".so" && e.path().extension() != ".dylib") continue;
        void* mod=dlopen(e.path().c_str(),RTLD_NOW); if(!mod) continue;
        auto create=reinterpret_cast<CreatePluginFn>(dlsym(mod,"core_create_plugin")); if(!create){dlclose(mod);continue;}
        auto* plugin=create(); if(!plugin){dlclose(mod);continue;}
#endif
        if(!plugin->initialize(api_)){
            plugin->shutdown(); delete plugin;
#if defined(_WIN32)
            FreeLibrary(mod);
#else
            dlclose(mod);
#endif
            continue;
        }
        loaded_.push_back({plugin->id(),plugin,mod});
    }
    return loaded_.size();
}
std::vector<std::string> PluginManager::loadedIds() const { std::vector<std::string> r; for(const auto& p:loaded_) r.push_back(p.id); return r; }
}
