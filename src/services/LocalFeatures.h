#pragma once
#include <filesystem>
#include <string>
#include "core/Result.h"
#include "core/RuntimePaths.h"
#include "core/Database.h"

namespace coreai::features {
class LocalFeatures {
public:
    LocalFeatures(const RuntimePaths& paths, Database& db);
    Result<std::string> libraryAdd(const std::string& name, const std::string& content);
    Result<std::string> libraryList() const;
    Result<std::string> fileWrite(const std::string& relative, const std::string& content);
    Result<std::string> fileRead(const std::string& relative) const;
    Result<std::string> documentInspect(const std::string& relative) const;
    Result<std::string> taskCreate(const std::string& name);
    Result<std::string> taskList() const;
    Result<std::string> workflowCreate(const std::string& name, const std::string& definition);
    Result<std::string> workflowList() const;
    Result<std::string> workflowRun(const std::string& id);
    Result<std::string> scheduleCreate(const std::string& name, long long delaySeconds);
    Result<std::string> scheduleList() const;
    Result<std::string> scheduleRun(const std::string& id);
    Result<std::string> agentRun(const std::string& request);
    Result<std::string> multiAgentRun(const std::string& request);
    Result<std::string> pluginDiscover() const;
    Result<std::string> gitStatus(const std::filesystem::path& root) const;
    Result<std::string> researchLocal(const std::string& query) const;
    Result<std::string> imageCreate(const std::string& name, const std::string& prompt);
    Result<std::string> mapSearch(const std::string& place) const;
    Result<std::string> directions(const std::string& from,const std::string& to) const;
private:
    const RuntimePaths& paths_;
    Database& db_;
    static std::string sql(const std::string& v);
    static std::string json(const std::string& v);
    static std::string makeId();
    static std::string sha256ish(const std::string& data);
    Result<std::filesystem::path> safe(const std::filesystem::path& base, const std::string& relative) const;
};
}
