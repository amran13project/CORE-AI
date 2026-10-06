#include "services/CoreApp.h"
#include <iostream>
#include <sstream>
#include <filesystem>
#ifdef COREAI_HAS_NATIVE_GUI
int runNativeGui(coreai::CoreApp& app);
#endif
int main(int argc,char**argv){
    std::string root; int idx=1;
    if(argc>2 && std::string(argv[1])=="--root"){root=argv[2];idx=3;}
    coreai::CoreApp app;auto init=app.initialize(root);if(!init.ok()){std::cerr<<"CORE-AI initialization failed: "<<init.error().message<<"\n";return 2;}
    std::string cmd=idx<argc?argv[idx]:"";
    if(cmd=="version"){std::cout<<"CORE-AI 0.3.0\n";return 0;}
    if(cmd=="status"){auto r=app.status();std::cout<<(r.ok()?r.value():r.error().message)<<"\n";return r.ok()?0:3;}
    if(cmd=="capabilities"){std::cout<<app.capabilities()<<"\n";return 0;}
    if(cmd=="doctor"){std::cout<<app.doctor()<<"\n";return 0;}
    if(cmd=="models"){auto r=app.models();if(!r.ok()){std::cerr<<r.error().message<<"\n";return 4;}std::cout<<r.value()<<"\n";return 0;}
    if(cmd=="new-chat"){auto r=app.newChat();if(!r.ok()){std::cerr<<r.error().message<<"\n";return 3;}std::cout<<r.value()<<"\n";return 0;}
    if(cmd=="reset-chat"){auto r=app.resetChat();if(!r.ok()){std::cerr<<r.error().message<<"\n";return 3;}std::cout<<r.value()<<"\n";return 0;}
    if(cmd=="recent"){auto r=app.recent();if(!r.ok()){std::cerr<<r.error().message<<"\n";return 3;}std::cout<<r.value()<<"\n";return 0;}
    if(cmd=="memory"&&idx+2<argc&&std::string(argv[idx+1])=="add"){auto r=app.remember(argv[idx+2]);if(!r.ok()){std::cerr<<r.error().message<<"\n";return 3;}std::cout<<r.value()<<"\n";return 0;}
    if(cmd=="memory"&&idx+2<argc&&std::string(argv[idx+1])=="search"){auto r=app.memorySearch(argv[idx+2]);if(!r.ok()){std::cerr<<r.error().message<<"\n";return 3;}std::cout<<r.value()<<"\n";return 0;}
    if(cmd=="projects"&&idx+2<argc&&std::string(argv[idx+1])=="create"){auto r=app.projectCreate(argv[idx+2]);if(!r.ok()){std::cerr<<r.error().message<<"\n";return 3;}std::cout<<r.value()<<"\n";return 0;}
    if(cmd=="projects"&&idx+1<argc&&std::string(argv[idx+1])=="list"){auto r=app.projectList();if(!r.ok()){std::cerr<<r.error().message<<"\n";return 3;}std::cout<<r.value()<<"\n";return 0;}
    if((cmd=="chat"||cmd=="think"||cmd=="codex")&&idx+1<argc){std::ostringstream q;for(int i=idx+1;i<argc;i++){if(i>idx+1)q<<' ';q<<argv[i];}auto r=app.chat(q.str(),cmd);if(!r.ok()){std::cerr<<r.error().message<<"\n";return 4;}std::cout<<r.value()<<"\n";return 0;}
    if(cmd=="serve"){auto exe=std::filesystem::weakly_canonical(std::filesystem::path(argv[0]));
        auto web=exe.parent_path().parent_path()/"web";
        if(!std::filesystem::exists(web)) web=exe.parent_path()/"web";
        auto r=app.startApi(web);if(!r.ok()){std::cerr<<r.error().message<<"\n";return 5;}std::cout<<"CORE-AI API: http://127.0.0.1:47821/\n";std::cout<<"Press Enter to stop.\n";std::string x;std::getline(std::cin,x);app.stopApi();return 0;}
#ifdef COREAI_HAS_NATIVE_GUI
    if(cmd=="gui"||cmd.empty())return runNativeGui(app);
#else
    if(cmd.empty()){std::cout<<"CORE-AI 0.3.0\nCommands: version status capabilities new-chat reset-chat recent chat think codex memory projects serve\n";return 0;}
#endif
    std::cerr<<"Unknown command\n";return 1;
}
