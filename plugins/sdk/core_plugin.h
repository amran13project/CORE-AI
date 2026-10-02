#pragma once
#ifdef _WIN32
#define CORE_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#else
#define CORE_PLUGIN_EXPORT extern "C" __attribute__((visibility("default")))
#endif
CORE_PLUGIN_EXPORT const char* core_plugin_name();
CORE_PLUGIN_EXPORT const char* core_plugin_version();
CORE_PLUGIN_EXPORT unsigned core_plugin_api();
