#include "core/CoreApp.h"
#include "server/LocalApiServer.h"
#include <iostream>
#include <string>
#include <sstream>
int main(int argc,char**argv){
 core::CoreApp app; if(!app.initialize(".")){std::cerr<<"CORE initialization failed\n";return 2;}
 if(argc>1){std::string cmd=argv[1];if(cmd=="status"){std::cout<<app.status()<<"\n";return 0;}if(cmd=="capabilities"){std::cout<<app.capabilities()<<"\n";return 0;}if((cmd=="chat"||cmd=="think"||cmd=="code"||cmd=="agent")&&argc>2){std::ostringstream q;for(int i=2;i<argc;i++){if(i>2)q<<' ';q<<argv[i];}auto p=q.str();if(cmd=="chat")std::cout<<app.chat(p);else if(cmd=="think")std::cout<<app.think(p);else if(cmd=="code")std::cout<<app.code(p);else std::cout<<app.agent(p);std::cout<<'\n';return 0;}}
 core::LocalApiServer server;server.start(app,47821,"gui");std::cout<<"CORE AI 0.2\nGUI: http://127.0.0.1:47821/\nPress Ctrl+C to stop.\n";server.run();return 0;
}
