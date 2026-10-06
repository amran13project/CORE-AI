#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include "core/Result.h"
struct sqlite3;
struct sqlite3_stmt;
namespace coreai {
struct Row { std::vector<std::string> values; };
class Database {
public:
    ~Database();
    Result<void> open(const std::filesystem::path& path);
    void close();
    Result<void> exec(const std::string& sql);
    Result<std::vector<Row>> query(const std::string& sql, const std::vector<std::string>& params = {}) const;
    bool available() const { return db_ != nullptr; }
    std::string lastError() const { return last_error_; }
private:
    void* module_{nullptr};
    sqlite3* db_{nullptr};
    std::string last_error_;
    struct Api;
    Api* api_{nullptr};
};
}
