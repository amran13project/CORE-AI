#pragma once
#include <string>
namespace core::adapters::image { class ImageGenerationAdapter { public: std::string providerContract()const{return "Stable Diffusion-compatible image provider required";} }; }
