#include "projects/ProjectManager.h"
#include <filesystem>
#include <fstream>
namespace core::projects {
Project ProjectManager::open(const std::string& root) const { auto p=std::filesystem::path(root); return {p.filename().string(),p.string()}; }
bool ProjectManager::create(const std::string& root,const std::string& name) const { auto p=std::filesystem::path(root); std::filesystem::create_directories(p/".core"); std::ofstream(p/".core"/"project.txt")<<"name="<<name<<'\n'; return true; }
}
