#pragma once
#include <string>
namespace core::verification {
struct VerificationRecord { std::string name; int exitCode=-1; bool passed=false; std::string output; };
class VerificationEngine {
public:
    VerificationRecord commandResult(std::string name, int exitCode, std::string output) const;
};
}
