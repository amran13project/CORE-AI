#pragma once
#include <string>
namespace core::plugins {
class CoreAPI {
public:
    virtual ~CoreAPI() = default;
    virtual void log(const std::string& message)=0;
};
class ICorePlugin {
public:
    virtual ~ICorePlugin()=default;
    virtual const char* id() const=0;
    virtual const char* name() const=0;
    virtual bool initialize(CoreAPI& core)=0;
    virtual void shutdown()=0;
};
using CreatePluginFn = ICorePlugin* (*)();
}
