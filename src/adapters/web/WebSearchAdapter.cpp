#include "adapters/web/WebSearchAdapter.h"
#include <cctype>
#include <iomanip>
#include <sstream>
namespace core::adapters::web { std::string googleUrl(const std::string&q){std::ostringstream o;for(unsigned char c:q){if(std::isalnum(c)||c=='-'||c=='_'||c=='.'||c=='~')o<<(char)c;else o<<'%'<<std::uppercase<<std::hex<<std::setw(2)<<std::setfill('0')<<(int)c<<std::dec;}return "https://www.google.com/search?q="+o.str();} }
