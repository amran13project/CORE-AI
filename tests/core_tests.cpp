#include "core/Config.h"
#include "memory/MemoryStore.h"
#include "security/PermissionManager.h"
#include "verification/VerificationEngine.h"
#include "agents/Planner.h"
#include "projects/ProjectManager.h"
#include <cassert>
#include <filesystem>
#include <iostream>
int main(){auto d=std::filesystem::temp_directory_path()/"core_ai_v02_tests";std::filesystem::remove_all(d);std::filesystem::create_directories(d);core::Config c;c.set("model","test");assert(c.get("model")=="test");assert(c.save((d/"c").string()));core::memory::MemoryStore m((d/"m").string());assert(m.add("personal","hello core"));assert(!m.search("hello").empty());core::security::PermissionManager p;assert(p.allows("memory.write"));assert(!p.allows("process.execute"));p.grant("process.execute");assert(p.allows("process.execute"));core::verification::VerificationEngine v;assert(v.commandResult(0,"ok").passed);assert(!v.commandResult(1,"bad").passed);core::agents::Planner pl;assert(pl.plan("test").size()==5);core::projects::ProjectManager pm((d/"projects").string());assert(pm.create("Demo"));assert(!pm.list().empty());std::cout<<"CORE tests passed\n";return 0;}
