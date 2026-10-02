#pragma once
#include "ai/AIProvider.h"
#include <memory>
#include <vector>
namespace core::ai { class ModelRouter { std::vector<std::unique_ptr<IAIProvider>> p_; public: void add(std::unique_ptr<IAIProvider>); IAIProvider* select() const; std::vector<std::string> ids() const; }; }
