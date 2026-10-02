#pragma once
#include <cstddef>
#include <string>
#include <vector>
namespace core::memory { struct Memory { long long id; std::string scope; std::string text; }; class MemoryStore { std::string p_; long long next_=1; public: explicit MemoryStore(std::string); bool add(std::string,std::string); std::vector<Memory> search(const std::string&,std::size_t=10) const; }; }
