#include "ai/ModelRouter.h"
namespace core::ai {
void ModelRouter::add(std::unique_ptr<IAIProvider> provider) { providers_.push_back(std::move(provider)); }
IAIProvider* ModelRouter::select(bool) const { return providers_.empty() ? nullptr : providers_.front().get(); }
std::vector<std::string> ModelRouter::providerIds() const { std::vector<std::string> r; for (const auto& p : providers_) r.emplace_back(p->id()); return r; }
}
