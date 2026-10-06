#pragma once
#include <filesystem>
#include <map>
#include <string>
#include "core/Result.h"
namespace coreai {
class Config {
public:
    Result<void> load(const std::filesystem::path& path);
    Result<void> save(const std::filesystem::path& path) const;
    void set(std::string k,std::string v); std::string get(const std::string& k,const std::string& d={}) const;
    bool boolean(const std::string& k,bool d) const;
private: std::map<std::string,std::string> values_;
};
}
