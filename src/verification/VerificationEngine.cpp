#include "verification/VerificationEngine.h"
namespace core::verification {
VerificationRecord VerificationEngine::commandResult(std::string name,int exitCode,std::string output) const { return {std::move(name),exitCode,exitCode==0,std::move(output)}; }
}
