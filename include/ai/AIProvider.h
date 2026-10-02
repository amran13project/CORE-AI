#pragma once
#include <string>
namespace core::ai { struct AIRequest { std::string system; std::string prompt; }; struct AIResult { bool ok=false; std::string text; std::string error; }; class IAIProvider { public: virtual ~IAIProvider()=default; virtual std::string id() const=0; virtual AIResult complete(const AIRequest&)=0; virtual bool available() const=0; }; }
