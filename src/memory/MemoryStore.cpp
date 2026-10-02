#include "memory/MemoryStore.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <chrono>

namespace {
std::string lower(std::string s) { std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); }); return s; }
std::string clean(std::string s) { for (char& c : s) if (c=='\t' || c=='\n' || c=='\r') c=' '; return s; }
}
namespace core::memory {
MemoryStore::MemoryStore(std::string path) : path_(std::move(path)) {}
void MemoryStore::add(std::string scope, std::string text) {
    std::ofstream out(path_, std::ios::app);
    if (!out) throw std::runtime_error("Cannot open memory: " + path_);
    const auto id = std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    out << id << '\t' << clean(scope) << '\t' << clean(text) << '\n';
}
std::vector<MemoryEntry> MemoryStore::all(size_t limit) const {
    std::ifstream in(path_); std::vector<MemoryEntry> result; std::string line;
    while (std::getline(in, line) && result.size() < limit) {
        std::stringstream ss(line); MemoryEntry m;
        std::getline(ss,m.id,'\t'); std::getline(ss,m.scope,'\t'); std::getline(ss,m.text);
        if (!m.id.empty()) result.push_back(std::move(m));
    }
    return result;
}
std::vector<MemoryEntry> MemoryStore::search(const std::string& query, size_t limit) const {
    const auto q = lower(query); std::vector<MemoryEntry> result; std::ifstream in(path_); std::string line;
    while (std::getline(in,line) && result.size() < limit) {
        std::stringstream ss(line); MemoryEntry m;
        std::getline(ss,m.id,'\t'); std::getline(ss,m.scope,'\t'); std::getline(ss,m.text);
        if (lower(m.scope + " " + m.text).find(q) != std::string::npos) result.push_back(std::move(m));
    }
    return result;
}
}
