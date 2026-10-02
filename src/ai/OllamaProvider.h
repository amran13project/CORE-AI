#pragma once
#include "ai/AIProvider.h"
#include <string>
namespace core::ai {
class OllamaProvider final : public IAIProvider {
public:
    explicit OllamaProvider(std::string model);
    const char* id() const override { return "ollama-cli"; }
    AIResponse complete(const AIRequest& request) override;
    const std::string& model() const { return model_; }
private:
    std::string model_;
};
}
