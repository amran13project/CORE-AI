#pragma once
#include <filesystem>
#include <string>
#include "core/Result.h"
#include "core/UserStorage.h"

namespace coreai {
class RuntimePaths {
public:
    static Result<RuntimePaths> resolve(const std::string& override_root = {});
    const std::filesystem::path& root() const { return root_; }
    const std::filesystem::path& config() const { return config_; }
    const std::filesystem::path& data() const { return data_; }
    const UserStorageInfo& userStorage() const { return user_storage_; }
    const std::filesystem::path& logs() const { return logs_; }
    const std::filesystem::path& temp() const { return temp_; }
    const std::filesystem::path& projects() const { return projects_; }
    const std::filesystem::path& library() const { return library_; }
    const std::filesystem::path& images() const { return images_; }
    const std::filesystem::path& backups() const { return backups_; }
    const std::filesystem::path& plugins() const { return plugins_; }
    const std::filesystem::path& web() const { return web_; }
private:
    std::filesystem::path root_, config_, data_, logs_, temp_, projects_, library_, images_, backups_, plugins_, web_;
    UserStorageInfo user_storage_;
};
}
