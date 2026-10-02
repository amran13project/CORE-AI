#pragma once
#include "plugins/PluginAPI.h"
#include <string>
#include <vector>
namespace core::plugins {
class PluginManager {
public:
    explicit PluginManager(CoreAPI& api);
    ~PluginManager();
    size_t discover(const std::string& directory);
    std::vector<std::string> loadedIds() const;
private:
    struct Loaded { std::string id; ICorePlugin* plugin=nullptr; void* module=nullptr; };
    CoreAPI& api_;
    std::vector<Loaded> loaded_;
};
}
