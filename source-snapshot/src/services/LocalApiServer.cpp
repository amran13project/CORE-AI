#include "services/LocalApiServer.h"
#include "services/CoreApp.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib")
using sock_t=SOCKET; static void close_sock(sock_t s){closesocket(s);} static bool good(sock_t s){return s!=INVALID_SOCKET;}
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
using sock_t=int; static void close_sock(sock_t s){close(s);} static bool good(sock_t s){return s>=0;}
#endif
namespace coreai {
static std::string esc(const std::string&s){std::string o;for(char c:s){if(c=='"')o+="\\\"";else if(c=='\\')o+="\\\\";else if(c=='\n')o+="\\n";else o+=c;}return o;}
static std::string urlDecode(const std::string& s){std::string o;for(size_t i=0;i<s.size();++i){if(s[i]=='%'&&i+2<s.size()){auto h=[](char c)->int{if(c>='0'&&c<='9')return c-'0';if(c>='A'&&c<='F')return c-'A'+10;if(c>='a'&&c<='f')return c-'a'+10;return -1;};int a=h(s[i+1]),b=h(s[i+2]);if(a>=0&&b>=0){o.push_back(char((a<<4)|b));i+=2;continue;}}o.push_back(s[i]=='+'?' ':s[i]);}return o;}
static std::string field(const std::string&j,const std::string&k){auto p=j.find("\""+k+"\":\"");if(p==std::string::npos)return{};p+=k.size()+4;std::string out;bool escp=false;for(;p<j.size();++p){char c=j[p];if(escp){out+=c;escp=false;continue;}if(c=='\\'){escp=true;continue;}if(c=='"')break;out+=c;}return out;}
static std::string resp(int code,const std::string&type,const std::string&body){return "HTTP/1.1 "+std::to_string(code)+(code==200?" OK":code==404?" Not Found":code==403?" Forbidden":" Error")+"\r\nContent-Type: "+type+"\r\nContent-Length: "+std::to_string(body.size())+"\r\nAccess-Control-Allow-Origin: http://127.0.0.1:47821\r\nConnection: close\r\n\r\n"+body;}
LocalApiServer::LocalApiServer(CoreApp&a):app_(a){} LocalApiServer::~LocalApiServer(){stop();}
Result<void> LocalApiServer::start(int p,const std::filesystem::path& root){
    if(running_) return Result<void>::success();
    if(root.empty() || !std::filesystem::exists(root) || !std::filesystem::is_directory(root))
        return Result<void>::failure(error(ErrorCode::StorageOpenFailed,"web root unavailable","api","start",true));
    port_=p;
    web_root_=std::filesystem::weakly_canonical(root);
    running_=true;
    thread_=std::thread(&LocalApiServer::runLoop,this);
    return Result<void>::success();
}
void LocalApiServer::stop(){if(!running_)return;running_=false; // connect to self to wake accept
#ifdef _WIN32
WSADATA w{};WSAStartup(MAKEWORD(2,2),&w);SOCKET s=socket(AF_INET,SOCK_STREAM,0);sockaddr_in a{};a.sin_family=AF_INET;a.sin_addr.s_addr=htonl(INADDR_LOOPBACK);a.sin_port=htons((u_short)port_);connect(s,(sockaddr*)&a,sizeof(a));closesocket(s);WSACleanup();
#else
int s=socket(AF_INET,SOCK_STREAM,0);sockaddr_in a{};a.sin_family=AF_INET;a.sin_addr.s_addr=htonl(INADDR_LOOPBACK);a.sin_port=htons((uint16_t)port_);connect(s,(sockaddr*)&a,sizeof(a));close(s);
#endif
if(thread_.joinable())thread_.join();}
void LocalApiServer::runLoop(){
#ifdef _WIN32
WSADATA w{};if(WSAStartup(MAKEWORD(2,2),&w)!=0){running_=false;return;}
#endif
sock_t s=socket(AF_INET,SOCK_STREAM,0);if(!good(s)){running_=false;return;}int one=1;setsockopt(s,SOL_SOCKET,SO_REUSEADDR,(char*)&one,sizeof(one));sockaddr_in a{};a.sin_family=AF_INET;a.sin_addr.s_addr=htonl(INADDR_LOOPBACK);a.sin_port=htons((uint16_t)port_);if(bind(s,(sockaddr*)&a,sizeof(a))<0){close_sock(s);running_=false;return;}if(listen(s,8)<0){close_sock(s);running_=false;return;}while(running_){sockaddr_in c{};
#ifdef _WIN32
int cl=sizeof(c);
#else
socklen_t cl=sizeof(c);
#endif
sock_t cs=accept(s,(sockaddr*)&c,&cl);if(!good(cs))continue;if(running_)handle(cs);else close_sock(cs);}close_sock(s);
#ifdef _WIN32
WSACleanup();
#endif
}
void LocalApiServer::handle(int s){std::string req;char b[8192];for(int i=0;i<16;i++){int n=recv(s,b,sizeof(b),0);if(n<=0)break;req.append(b,n);if(req.find("\r\n\r\n")!=std::string::npos)break;}auto e=req.find("\r\n");if(e==std::string::npos){auto x=resp(400,"text/plain","bad request");send(s,x.data(),x.size(),0);close_sock(s);return;}std::istringstream first(req.substr(0,e));std::string method,path,ver;first>>method>>path>>ver;auto hh=req.find("\r\n\r\n");std::string body=hh==std::string::npos?"":req.substr(hh+4);std::string out;int code=200;std::string type="application/json";
if(path=="/health")out="{\"status\":\"OK\"}";else if(path=="/ready"){auto s1=app_.status();out=s1.ok()?s1.value():"{\"status\":\"FAILED\"}";}else if(path=="/version"){out="{\"version\":\"0.3.0\"}";}else if(path=="/status"){auto r=app_.status();if(r.ok())out=r.value();else{code=500;out="{\"error\":\"status failed\"}";}}else if(path=="/capabilities")out=app_.capabilities();else if(path=="/chat"&&method=="POST"){auto mode=field(body,"mode");auto prompt=field(body,"prompt");auto r=app_.chat(prompt,mode.empty()?"chat":mode);if(r.ok())out="{\"text\":\""+esc(r.value())+"\"}";else{code=r.error().code==ErrorCode::ProviderUnavailable?503:400;out="{\"error\":\""+esc(r.error().message)+"\"}";}}else if(path=="/conversations/new"&&method=="POST"){auto r=app_.newChat();if(r.ok())out="{\"id\":\""+esc(r.value())+"\"}";else{code=500;out="{\"error\":\""+esc(r.error().message)+"\"}";}}else if(path=="/conversations/reset"&&method=="POST"){auto r=app_.resetChat();if(r.ok())out="{\"id\":\""+esc(r.value())+"\"}";else{code=500;out="{\"error\":\""+esc(r.error().message)+"\"}";}}else if(path=="/conversations/recent"){auto r=app_.recent();if(r.ok())out=r.value();else{code=500;out="{\"error\":\""+esc(r.error().message)+"\"}";}}else if(path=="/memory"&&method=="POST"){auto r=app_.remember(field(body,"text"),field(body,"scope").empty()?"personal":field(body,"scope"));if(r.ok())out="{\"id\":\""+esc(r.value())+"\"}";else{code=400;out="{\"error\":\""+esc(r.error().message)+"\"}";}}else if(path.rfind("/memory/search",0)==0){auto qpos=path.find("q=");auto q=qpos==std::string::npos?"":urlDecode(path.substr(qpos+2));auto r=app_.memorySearch(q);if(r.ok())out=r.value();else code=500;}else if(path=="/projects"&&method=="GET"){auto r=app_.projectList();if(r.ok())out=r.value();else code=500;}else if(path=="/projects"&&method=="POST"){auto r=app_.projectCreate(field(body,"name"));if(r.ok())out="{\"id\":\""+esc(r.value())+"\"}";else{code=400;out="{\"error\":\""+esc(r.error().message)+"\"}";}}else if(path=="/models"){auto r=app_.models();if(r.ok())out=r.value();else{code=503;out="{\"status\":\"UNAVAILABLE\",\"reason\":\""+esc(r.error().message)+"\"}";}}
else if(path=="/doctor"){out=app_.doctor();}
else if(path=="/library"||path=="/files"||path=="/documents"||path=="/git"||path=="/research"||path=="/maps"||path=="/images"||path=="/voice"||path=="/vision"||path=="/multimodal"||path=="/tools"||path=="/agents"||path=="/workflows"||path=="/scheduled"||path=="/plugins"||path=="/tasks"){code=501;out="{\"status\":\"NOT_IMPLEMENTED\",\"reason\":\"This service is not implemented in the current build\"}";}else if(method=="GET"){std::filesystem::path f=(path=="/"?"/index.html":path);if(f.string().find("..")!=std::string::npos){code=403;type="text/plain";out="forbidden";}else{auto p=std::filesystem::path(web_root_)/f.string().substr(1);std::ifstream in(p,std::ios::binary);if(!in){code=404;type="text/plain";out="not found";}else{std::ostringstream ss;ss<<in.rdbuf();out=ss.str();if(p.extension()==".html")type="text/html";else if(p.extension()==".css")type="text/css";else if(p.extension()==".js")type="application/javascript";}}}else{code=404;out="{\"error\":\"not found\"}";}auto x=resp(code,type,out);send(s,x.data(),(int)x.size(),0);close_sock(s);}
}
