#include "ui/ConsoleUI.h"
#include <iostream>
#include <sstream>
namespace core::ui {
int runConsole(CoreApp& app,int argc,char** argv){
    if(argc<2){std::cout<<"CORE AI\nCommands: chat, ask, memory, tool, agent, doctor, capabilities\n"; return 0;}
    const std::string cmd=argv[1];
    if(cmd=="chat"||cmd=="ask"){
        std::ostringstream q; for(int i=2;i<argc;++i){if(i>2)q<<' ';q<<argv[i];}
        std::cout<<app.chat(q.str())<<'\n'; return 0;
    }
    if(cmd=="memory"&&argc>=4&&std::string(argv[2])=="add"){std::ostringstream q;for(int i=3;i<argc;++i){if(i>3)q<<' ';q<<argv[i];}app.addMemory("personal",q.str());return 0;}
    if(cmd=="memory"&&argc>=4&&std::string(argv[2])=="search"){std::ostringstream q;for(int i=3;i<argc;++i){if(i>3)q<<' ';q<<argv[i];}std::cout<<app.searchMemory(q.str())<<'\n';return 0;}
    if(cmd=="agent"){std::ostringstream q;for(int i=2;i<argc;++i){if(i>2)q<<' ';q<<argv[i];}std::cout<<app.runAgent(q.str())<<'\n';return 0;}
    if(cmd=="doctor"){std::cout<<app.doctor()<<'\n';return 0;}
    if(cmd=="capabilities"){std::cout<<app.capabilities()<<'\n';return 0;}
    if(cmd=="tool"&&argc>=5){std::cout<<app.tool(argv[2],argv[3],argv[4])<<'\n';return 0;}
    std::cout<<"Unknown command. Run without arguments for help.\n"; return 1;
}
}
