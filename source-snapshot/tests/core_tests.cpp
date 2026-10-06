#include "services/CoreApp.h"
#include <cassert>
#include <filesystem>
#include <iostream>
int main(){auto root=std::filesystem::temp_directory_path()/"core-ai-test-isolated";std::filesystem::remove_all(root);coreai::CoreApp app;auto r=app.initialize(root.string());assert(r.ok());assert(app.db().available());auto c=app.newChat();assert(c.ok()&&!c.value().empty());auto m=app.remember("CORE test memory","test");assert(m.ok());auto ms=app.memorySearch("CORE test");assert(ms.ok()&&ms.value().find("CORE test memory")!=std::string::npos);auto p=app.projectCreate("Demo");assert(p.ok());auto ps=app.projectList();assert(ps.ok()&&ps.value().find("Demo")!=std::string::npos);auto rec=app.recent();assert(rec.ok()&&rec.value().find(c.value())!=std::string::npos);auto reset=app.resetChat();assert(reset.ok()&&reset.value()!=c.value());std::cout<<"CORE-AI tests passed\n";return 0;}
