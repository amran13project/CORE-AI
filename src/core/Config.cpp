#include "core/Config.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace core {
void Config::load(const std::string& path) {
    values_.clear();
    std::ifstream in(path);
    if (!in) return;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        const auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        values_[line.substr(0, eq)] = line.substr(eq + 1);
    }
}
void Config::save(const std::string& path) const {
    std::ofstream out(path, std::ios::trunc);
    if (!out) throw std::runtime_error("Cannot write config: " + path);
    for (const auto& [k,v] : values_) out << k << '=' << v << '\n';
}
std::string Config::get(const std::string& key, const std::string& fallback) const {
    auto it = values_.find(key); return it == values_.end() ? fallback : it->second;
}
void Config::set(const std::string& key, const std::string& value) { values_[key] = value; }
}
