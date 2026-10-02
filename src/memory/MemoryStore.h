#pragma once
#include <string>
#include <vector>
namespace core::memory {
struct MemoryEntry { std::string id; std::string scope; std::string text; };
class MemoryStore {
public:
    explicit MemoryStore(std::string path);
    void add(std::string scope, std::string text);
    std::vector<MemoryEntry> search(const std::string& query, size_t limit = 10) const;
    std::vector<MemoryEntry> all(size_t limit = 100) const;
private:
    std::string path_;
};
}
