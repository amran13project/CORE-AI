#include "agents/Planner.h"
namespace core::agents { std::vector<Step> Planner::plan(const std::string&r)const{return {{1,"Understand: "+r},{2,"Choose tools/providers"},{3,"Execute with permission checks"},{4,"Verify result"},{5,"Report evidence"}};} }
