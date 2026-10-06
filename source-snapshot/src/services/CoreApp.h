#pragma once
#include <filesystem>
#include <memory>
#include <string>
#include "core/RuntimePaths.h"
#include "core/Logger.h"
#include "core/Config.h"
#include "core/Database.h"
#include "core/Capabilities.h"
#include "providers/OllamaProvider.h"
#include "core/Result.h"
namespace coreai {
class LocalApiServer;
class CoreApp {
public:
    CoreApp(); ~CoreApp();
    Result<void> initialize(const std::string& runtime_root = {});
    Result<std::string> newChat();
    Result<std::string> resetChat();
    Result<std::string> chat(const std::string& prompt, const std::string& mode="chat");
    Result<std::string> recent() const;
    Result<std::string> status() const;
    std::string doctor() const;
    Result<std::string> models() const;
    std::string capabilities() const { return caps_.json(); }
    Result<std::string> remember(const std::string& text,const std::string& scope="personal");
    Result<std::string> memorySearch(const std::string& q) const;
    Result<std::string> projectCreate(const std::string& name);
    Result<std::string> projectList() const;
    Result<void> setModel(const std::string& model);
    std::string selectedModel()const{return selected_model_;}
    const RuntimePaths& paths()const{return paths_;}
    bool initialized()const{return initialized_;}
    const Database& db()const{return db_;}
    int apiPort()const{return 47821;}
    Result<void> startApi(const std::filesystem::path& web_root);
    void stopApi();
private:
    Result<void> setupSchema();
    void buildCapabilities();
    Result<std::string> ensureConversation();
    std::string current_conversation_;
    RuntimePaths paths_;
    std::unique_ptr<Logger> logger_;
    Config config_;
    Database db_;
    CapabilityRegistry caps_;
    std::unique_ptr<providers::OllamaProvider> ollama_;
    std::unique_ptr<LocalApiServer> api_;
    std::string selected_model_;
    bool initialized_{false};
};
}
