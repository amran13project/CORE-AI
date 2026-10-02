#include "verification/VerificationEngine.h"
namespace core::verification { Result VerificationEngine::commandResult(int c,const std::string&d)const{return {c==0,c,d};} }
