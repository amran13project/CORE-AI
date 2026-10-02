#pragma once
#include "ai/AIProvider.h"
#include <memory>
#include <vector>
namespace core::ai {
class ModelRouter {
public:
    void add(std::unique_ptr<IAIProvider> provider);
    IAIProvider* select(bool requiresLocal = true) const;
    std::vector<std::string> providerIds() const;
private:
    std::vector<std::unique_ptr<IAIProvider>> providers_;
};
}
