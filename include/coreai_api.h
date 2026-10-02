#pragma once
#ifdef _WIN32
#define CORE_API extern "C" __declspec(dllexport)
#else
#define CORE_API extern "C" __attribute__((visibility("default")))
#endif
struct CoreHandle;
CORE_API CoreHandle* core_create(); CORE_API void core_destroy(CoreHandle*); CORE_API void core_free(char*);
CORE_API char* core_version(CoreHandle*); CORE_API char* core_status(CoreHandle*); CORE_API char* core_capabilities(CoreHandle*);
CORE_API char* core_chat(CoreHandle*, const char*); CORE_API char* core_think(CoreHandle*, const char*); CORE_API char* core_code(CoreHandle*, const char*); CORE_API char* core_agent(CoreHandle*, const char*);
CORE_API char* core_memory_search(CoreHandle*, const char*); CORE_API int core_remember(CoreHandle*, const char*, const char*);
CORE_API char* core_map_url(CoreHandle*, const char*); CORE_API char* core_directions_url(CoreHandle*, const char*, const char*);
