#pragma once
#include <cstdint>
#include <filesystem>
#include <string>
#include "core/Result.h"

namespace coreai {

struct UserStorageInfo {
    std::string user_id;
    std::filesystem::path users_root;
    std::filesystem::path user_root;
    std::filesystem::path shard_root;
    std::uint64_t shard_index{1};
    std::uintmax_t logical_quota_bytes{0};
};

class UserStorage {
public:
    static Result<UserStorageInfo> resolve(const std::filesystem::path& root,
                                           const std::string& user_id = {});
    static Result<UserStorageInfo> ensure_capacity(const UserStorageInfo& current,
                                                   std::uintmax_t incoming_bytes = 0);
    static std::uintmax_t directory_size(const std::filesystem::path& root);
private:
    static std::string sanitize(const std::string& value);
    static std::uintmax_t quota_bytes();
};

}
