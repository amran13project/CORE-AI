#pragma once
#include "agents/Planner.h"
#include <string>
namespace core::agents { class AgentManager { Planner p_; public: std::string planOnly(const std::string&) const; }; }
