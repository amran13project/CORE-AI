#pragma once
#include <atomic>
#include <string>
#include <filesystem>
#include <thread>
#include "core/Result.h"
namespace coreai { class CoreApp; class LocalApiServer {
public: explicit LocalApiServer(CoreApp& app); ~LocalApiServer(); Result<void> start(int port,const std::filesystem::path& web_root); void stop(); bool running()const{return running_;}
private: void runLoop(); void handle(int socket); CoreApp& app_; std::atomic_bool running_{false}; int port_{47821}; std::filesystem::path web_root_; std::thread thread_;
}; }
