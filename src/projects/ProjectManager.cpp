#include "projects/ProjectManager.h"
#include <filesystem>
#include <fstream>
namespace core::projects { namespace fs=std::filesystem; ProjectManager::ProjectManager(std::string r):root_(std::move(r)){fs::create_directories(root_);} bool ProjectManager::create(const std::string&n){fs::path p=fs::path(root_)/n;std::error_code e;fs::create_directories(p,e);if(e)return false;std::ofstream f(p/"project.json");f<<"{\"name\":\""<<n<<"\",\"version\":1}\n";return true;} std::vector<std::string> ProjectManager::list()const{std::vector<std::string>o;std::error_code e;for(auto&i:fs::directory_iterator(root_,e))if(i.is_directory())o.push_back(i.path().filename().string());return o;} }
