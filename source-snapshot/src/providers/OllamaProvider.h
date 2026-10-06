#pragma once
#include "providers/Provider.h"
namespace coreai::providers {
class OllamaProvider final: public Provider {
public:
    OllamaProvider(std::string url,std::string model);
    std::string id()const override{return "ollama";}
    Result<std::vector<Model>> discover() override;
    Result<GenerationResponse> generate(const GenerationRequest&) override;
    bool reachable()const override;
    std::string model()const{return model_;} std::string url()const{return url_;}
private: std::string url_,model_;
};
}
