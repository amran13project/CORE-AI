#pragma once
#include <string>

namespace core::ai {
struct AIRequest { std::string system; std::string prompt; };
struct AIResponse { bool ok=false; std::string text; std::string error; int exitCode=-1; };
class IAIProvider {
public:
    virtual ~IAIProvider() = default;
    virtual const char* id() const = 0;
    virtual AIResponse complete(const AIRequest& request) = 0;
};
}
