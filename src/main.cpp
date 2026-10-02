#include "core/CoreApp.h"
#include "ui/ConsoleUI.h"
#if defined(_WIN32) && CORE_HAS_WIN32_GUI
namespace core::ui { int runWin32(core::CoreApp& app); }
#endif
#include <filesystem>
#include <iostream>
int main(int argc,char**argv){
    core::CoreApp app;
    if(!app.initialize(".")){std::cerr<<"CORE initialization failed\n";return 2;}
#if defined(_WIN32) && CORE_HAS_WIN32_GUI
    if(argc==1) return core::ui::runWin32(app);
#endif
    return core::ui::runConsole(app,argc,argv);
}
