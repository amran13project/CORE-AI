#include "adapters/maps/MapsAdapter.h"
#include <cctype>
#include <iomanip>
#include <sstream>
namespace core::adapters::maps { static std::string e(const std::string&q){std::ostringstream o;for(unsigned char c:q){if(std::isalnum(c)||c=='-'||c=='_'||c=='.'||c=='~')o<<(char)c;else o<<'%'<<std::uppercase<<std::hex<<std::setw(2)<<std::setfill('0')<<(int)c<<std::dec;}return o.str();} std::string searchUrl(const std::string&p){return "https://www.google.com/maps/search/?api=1&query="+e(p);} std::string directionsUrl(const std::string&a,const std::string&b){return "https://www.google.com/maps/dir/?api=1&origin="+e(a)+"&destination="+e(b);} }
