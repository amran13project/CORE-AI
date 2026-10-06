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
    p.config_=p.root_/"config"; p.data_=p.root_/"data"; p.logs_=p.root_/"logs"; p.temp_=p.root_/"temp";
    p.projects_=p.root_/"projects"; p.library_=p.root_/"library"; p.images_=p.root_/"images";
    p.backups_=p.root_/"backups"; p.plugins_=p.root_/"plugins";
    p.web_=p.root_/"web";
    std::error_code ec;
    for (auto& d: {p.root_,p.config_,p.data_,p.logs_,p.temp_,p.projects_,p.library_,p.images_,p.backups_,p.plugins_}) fs::create_directories(d, ec);
    if (ec) return Result<RuntimePaths>::failure(error(ErrorCode::StorageOpenFailed, ec.message(), "runtime", "create_paths", true));
    return Result<RuntimePaths>::success(p);
}
}
