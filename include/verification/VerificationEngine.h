#pragma once
#include <string>
namespace core::verification { struct Result{bool passed;int code;std::string detail;}; class VerificationEngine{public: Result commandResult(int,const std::string&) const;}; }
