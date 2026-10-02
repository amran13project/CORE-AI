#include "agents/Planner.h"
#include <algorithm>
#include <cctype>
namespace core::agents {
std::vector<PlanStep> Planner::create(const std::string& request) const {
    std::string s=request; std::transform(s.begin(),s.end(),s.begin(),[](unsigned char c){return static_cast<char>(std::tolower(c));});
    std::vector<PlanStep> r{{1,"understand",request}};
    if(s.find("build")!=std::string::npos) r.push_back({2,"build","run the project's configured build command"});
    if(s.find("test")!=std::string::npos || s.find("fix")!=std::string::npos) r.push_back({3,"test","run the project's test command"});
    if(s.find("research")!=std::string::npos) r.push_back({2,"research","use the configured research/browser plugin"});
    r.push_back({static_cast<int>(r.size()+1),"verify","record actual exit codes and results"});
    return r;
}
}
