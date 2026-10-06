#pragma once
#ifdef _WIN32
#define COREAI_API extern "C" __declspec(dllexport)
#else
#define COREAI_API extern "C" __attribute__((visibility("default")))
#endif
struct CoreHandle;
COREAI_API CoreHandle* core_create(const char* runtime_root);
COREAI_API void core_destroy(CoreHandle*);
COREAI_API void core_free(char*);
COREAI_API char* core_version(CoreHandle*);
COREAI_API char* core_status(CoreHandle*);
COREAI_API char* core_capabilities(CoreHandle*);
COREAI_API char* core_chat(CoreHandle*, const char*);
