#include "core/Config.h"
#include "memory/MemoryStore.h"
#include "agents/Planner.h"
#include "verification/VerificationEngine.h"
#include <cassert>
#include <filesystem>
#include <iostream>
int main(){
    const auto dir=std::filesystem::temp_directory_path()/"core_ai_tests"; std::filesystem::remove_all(dir); std::filesystem::create_directories(dir);
    core::Config c; c.set("model","test"); assert(c.get("model")=="test");
    core::memory::MemoryStore m((dir/"mem.txt").string()); m.add("test","hello core"); assert(!m.search("core").empty());
    core::agents::Planner p; auto plan=p.create("build and test project"); assert(plan.size()>=3);
    core::verification::VerificationEngine v; auto ok=v.commandResult("test",0,"done"); assert(ok.passed); auto bad=v.commandResult("test",1,"bad"); assert(!bad.passed);
    std::cout<<"CORE tests passed\n"; return 0;
}
