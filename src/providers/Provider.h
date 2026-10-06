#pragma once
#include <string>
#include <vector>
#include "core/Result.h"
namespace coreai::providers {
struct Model { std::string provider,id,display; bool streaming=false,vision=false,tools=false; };
struct GenerationRequest { std::string system,prompt,model; };
struct GenerationResponse { std::string text; std::string provider,model; long long elapsed_ms=0; };
class Provider { public: virtual ~Provider()=default; virtual std::string id()const=0; virtual Result<std::vector<Model>> discover()=0; virtual Result<GenerationResponse> generate(const GenerationRequest&)=0; virtual bool reachable()const=0; };
}
