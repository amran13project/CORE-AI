#include "core/UserStorage.h"
#include <cstdlib>
#include <fstream>
#include <regex>
#include <sstream>

namespace coreai {
namespace fs = std::filesystem;

std::string UserStorage::sanitize(const std::string& value) {
    std::string out;
    for (char c : value) {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '-' || c == '_') out.push_back(c);
        else out.push_back('_');
    }
    if (out.empty()) out = "local-user";
    return out.substr(0, 80);
}

std::uintmax_t UserStorage::quota_bytes() {
    const char* env = std::getenv("CORE_USER_STORAGE_QUOTA_MB");
    std::uint64_t mb = 512;
    if (env && *env) {
        try { mb = std::stoull(env); } catch (...) { mb = 512; }
    }
    if (mb < 16) mb = 16;
    return static_cast<std::uintmax_t>(mb) * 1024ull * 1024ull;
}

std::uintmax_t UserStorage::directory_size(const fs::path& root) {
    std::uintmax_t total = 0;
    std::error_code ec;
    if (!fs::exists(root, ec)) return 0;
    for (fs::recursive_directory_iterator it(root, fs::directory_options::skip_permission_denied, ec), end;
         it != end && !ec; it.increment(ec)) {
        if (it->is_regular_file(ec)) {
            std::error_code sec;
            total += it->file_size(sec);
        }
    }
    return total;
}

Result<UserStorageInfo> UserStorage::resolve(const fs::path& root, const std::string& user_id) {
    UserStorageInfo info;
    info.user_id = sanitize(user_id.empty() ? (std::getenv("CORE_USER_ID") ? std::getenv("CORE_USER_ID") : "local-user") : user_id);
    info.users_root = root / "users";
    info.user_root = info.users_root / info.user_id;
    info.logical_quota_bytes = quota_bytes();
    std::error_code ec;
    fs::create_directories(info.user_root, ec);
    if (ec) return Result<UserStorageInfo>::failure(error(ErrorCode::StorageOpenFailed, ec.message(), "user-storage", "create_user_root", true));

    std::uint64_t best = 0;
    for (const auto& entry : fs::directory_iterator(info.user_root, ec)) {
        if (ec) break;
        if (!entry.is_directory()) continue;
        const auto name = entry.path().filename().string();
        if (name.rfind("shard-", 0) != 0) continue;
        try { best = std::max<std::uint64_t>(best, std::stoull(name.substr(6))); } catch (...) {}
    }
    if (best == 0) best = 1;
    info.shard_index = best;
    info.shard_root = info.user_root / (std::string("shard-") + (best < 10 ? "00" : best < 100 ? "0" : "") + std::to_string(best));
    fs::create_directories(info.shard_root, ec);
    if (ec) return Result<UserStorageInfo>::failure(error(ErrorCode::StorageOpenFailed, ec.message(), "user-storage", "create_shard", true));

    const auto used = directory_size(info.shard_root);
    if (used > info.logical_quota_bytes) {
        return ensure_capacity(info, 0);
    }
    return Result<UserStorageInfo>::success(info);
}

Result<UserStorageInfo> UserStorage::ensure_capacity(const UserStorageInfo& current,
                                                       std::uintmax_t incoming_bytes) {
    const auto used = directory_size(current.shard_root);
    if (used + incoming_bytes <= current.logical_quota_bytes) return Result<UserStorageInfo>::success(current);

    UserStorageInfo next = current;
    ++next.shard_index;
    next.shard_root = current.user_root / (std::string("shard-") +
        (next.shard_index < 10 ? "00" : next.shard_index < 100 ? "0" : "") + std::to_string(next.shard_index));
    std::error_code ec;
    fs::create_directories(next.shard_root, ec);
    if (ec) return Result<UserStorageInfo>::failure(error(ErrorCode::StorageOpenFailed, ec.message(), "user-storage", "rollover", true));
    return Result<UserStorageInfo>::success(next);
}

}
