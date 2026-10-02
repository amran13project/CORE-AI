#pragma once
#include <cstdint>
#include <string>
namespace core { class CoreApp; class LocalApiServer { CoreApp* app_=nullptr; std::uint16_t port_=47821; std::string webroot_; bool running_=false; public: bool start(CoreApp&,std::uint16_t,std::string); void run(); void stop(); }; }
