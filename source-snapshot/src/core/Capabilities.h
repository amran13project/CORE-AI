#pragma once
#include <string>
#include <vector>
namespace coreai {
struct Capability { std::string id,name,status,reason; bool implemented,available,configured,enabled; };
class CapabilityRegistry {
public:
    void set(Capability c); std::vector<Capability> all() const; std::string json() const;
private: std::vector<Capability> items_;
};
}
