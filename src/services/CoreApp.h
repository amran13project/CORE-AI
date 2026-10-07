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
#include "services/LocalFeatures.h"
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
    Result<std::string> libraryAdd(const std::string& name,const std::string& content);
    Result<std::string> libraryList() const;
    Result<std::string> fileWrite(const std::string& relative,const std::string& content);
    Result<std::string> fileRead(const std::string& relative) const;
    Result<std::string> documentInspect(const std::string& relative) const;
    Result<std::string> taskCreate(const std::string& name);
    Result<std::string> taskList() const;
    Result<std::string> workflowCreate(const std::string& name,const std::string& definition);
    Result<std::string> workflowList() const;
    Result<std::string> workflowRun(const std::string& id);
    Result<std::string> scheduleCreate(const std::string& name,long long delaySeconds);
    Result<std::string> scheduleList() const;
    Result<std::string> scheduleRun(const std::string& id);
    Result<std::string> agentRun(const std::string& request);
    Result<std::string> multiAgentRun(const std::string& request);
    Result<std::string> pluginDiscover() const;
    Result<std::string> gitStatus(const std::filesystem::path& root) const;
    Result<std::string> researchLocal(const std::string& query) const;
    Result<std::string> imageCreate(const std::string& name,const std::string& prompt);
    Result<std::string> mapSearch(const std::string& place) const;
    Result<std::string> directions(const std::string& from,const std::string& to) const;
    std::string selectedModel()const{return selected_model_;}
    const RuntimePaths& paths()const{return paths_;}
    bool initialized()const{return initialized_;}
    const Database& db()const{return db_;}
    int apiPort() const { const char* e=std::getenv("PORT"); if(!e||!*e)return 47821; char* end=nullptr; long p=std::strtol(e,&end,10); return (end==e||*end!="\0"||p<1||p>65535)?47821:(int)p; }
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
    std::unique_ptr<features::LocalFeatures> features_;
    std::unique_ptr<LocalApiServer> api_;
    std::string selected_model_;
    bool initialized_{false};
};
}
