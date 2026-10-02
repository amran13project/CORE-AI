#pragma once
#include "ai/AIProvider.h"
namespace core::ai { class OllamaProvider: public IAIProvider { std::string url_,model_; public: OllamaProvider(); explicit OllamaProvider(std::string); std::string id() const override{return "ollama";} AIResult complete(const AIRequest&) override; bool available() const override; }; }
