#include "tools/FileTool.h"
#include <fstream>
#include <sstream>
namespace core::tools { std::string readFile(const std::string&p){std::ifstream f(p,std::ios::binary);if(!f)return{};std::ostringstream o;o<<f.rdbuf();return o.str();} bool writeFile(const std::string&p,const std::string&s){std::ofstream f(p,std::ios::binary);if(!f)return false;f<<s;return (bool)f;} }
