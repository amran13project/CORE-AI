#include "core/RuntimePaths.h"
#include <cstdlib>
namespace fs = std::filesystem;
namespace coreai {
Result<RuntimePaths> RuntimePaths::resolve(const std::string& override_root) {
    RuntimePaths p;
    if (!override_root.empty()) p.root_ = fs::absolute(override_root);
    else {
#ifdef _WIN32
        const char* app = std::getenv("APPDATA");
        p.root_ = app ? fs::path(app) / "CORE-AI" : fs::current_path() / ".core-ai";
#else
        const char* home = std::getenv("HOME");
        p.root_ = home ? fs::path(home) / ".local" / "share" / "CORE-AI" : fs::current_path() / ".core-ai";
#endif
    }
    p.config_=p.root_/"config"; p.logs_=p.root_/"logs"; p.temp_=p.root_/"temp";
    auto us = UserStorage::resolve(p.root_);
    if (!us.ok()) return Result<RuntimePaths>::failure(us.error());
    p.user_storage_=us.value();
    p.data_=p.user_storage_.shard_root / "data";
    p.projects_=p.user_storage_.shard_root / "projects"; p.library_=p.user_storage_.shard_root / "library"; p.images_=p.user_storage_.shard_root / "images";
    p.backups_=p.user_storage_.shard_root / "backups"; p.plugins_=p.user_storage_.shard_root / "plugins";
    p.web_=p.root_/"web";
    std::error_code ec;
    for (auto& d: {p.root_,p.config_,p.data_,p.logs_,p.temp_,p.projects_,p.library_,p.images_,p.backups_,p.plugins_}) fs::create_directories(d, ec);
    if (ec) return Result<RuntimePaths>::failure(error(ErrorCode::StorageOpenFailed, ec.message(), "runtime", "create_paths", true));
    return Result<RuntimePaths>::success(p);
}
}
