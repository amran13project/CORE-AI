#pragma once
#include <string>
namespace core::adapters::vision { struct VisionRequest{std::string imagePath;std::string question;}; class VisionAdapter { public: std::string providerContract()const{return "vision-capable model provider required";} }; }
