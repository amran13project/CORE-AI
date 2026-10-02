#include "plugins/PluginAPI.h"
#include <iostream>
class ExamplePlugin final : public core::plugins::ICorePlugin {
public:
    const char* id() const override { return "example"; }
    const char* name() const override { return "CORE Example Plugin"; }
    bool initialize(core::plugins::CoreAPI& core) override { core.log("Example plugin initialized"); return true; }
    void shutdown() override { std::cout << "Example plugin shutdown\n"; }
};
extern "C" core::plugins::ICorePlugin* core_create_plugin(){ return new ExamplePlugin(); }
