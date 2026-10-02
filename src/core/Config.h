#pragma once
#include <string>
#include <unordered_map>

namespace core {
class Config {
public:
    void load(const std::string& path);
    void save(const std::string& path) const;
    std::string get(const std::string& key, const std::string& fallback = {}) const;
    void set(const std::string& key, const std::string& value);
private:
    std::unordered_map<std::string, std::string> values_;
};
}
