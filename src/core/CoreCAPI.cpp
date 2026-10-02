#include "coreai_api.h"
#include "core/CoreApp.h"
#include <cstdlib>
#include <cstring>
struct CoreHandle { core::CoreApp app; bool ready=false; CoreHandle(){ready=app.initialize(".");} };
static char* dup(const std::string&s){char*p=(char*)std::malloc(s.size()+1);if(!p)return nullptr;std::memcpy(p,s.data(),s.size());p[s.size()]=0;return p;}
#ifdef _WIN32
#define CORE_API extern "C" __declspec(dllexport)
#else
#define CORE_API extern "C" __attribute__((visibility("default")))
#endif
CORE_API CoreHandle* core_create(){return new CoreHandle();}
CORE_API void core_destroy(CoreHandle*h){delete h;}
CORE_API void core_free(char*p){std::free(p);}
CORE_API char* core_version(CoreHandle*){return dup("0.2.0");}
CORE_API char* core_status(CoreHandle*h){return dup(h?h->app.status():"{}");}
CORE_API char* core_capabilities(CoreHandle*h){return dup(h?h->app.capabilities():"");}
CORE_API char* core_chat(CoreHandle*h,const char*p){return dup(h&&p?h->app.chat(p):"CORE ERROR");}
CORE_API char* core_think(CoreHandle*h,const char*p){return dup(h&&p?h->app.think(p):"CORE ERROR");}
CORE_API char* core_code(CoreHandle*h,const char*p){return dup(h&&p?h->app.code(p):"CORE ERROR");}
CORE_API char* core_agent(CoreHandle*h,const char*p){return dup(h&&p?h->app.agent(p):"CORE ERROR");}
CORE_API char* core_memory_search(CoreHandle*h,const char*p){return dup(h&&p?h->app.memorySearch(p):"CORE ERROR");}
CORE_API int core_remember(CoreHandle*h,const char*t,const char*s){return h&&t&&h->app.remember(t,s?s:"personal")?1:0;}
CORE_API char* core_map_url(CoreHandle*h,const char*p){return dup(h&&p?h->app.mapUrl(p):"CORE ERROR");}
CORE_API char* core_directions_url(CoreHandle*h,const char*a,const char*b){return dup(h&&a&&b?h->app.directionsUrl(a,b):"CORE ERROR");}
