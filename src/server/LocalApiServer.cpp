#include "server/LocalApiServer.h"
#include "core/CoreApp.h"
#include "tools/FileTool.h"
#include <algorithm>
#include <filesystem>
#include <sstream>
#include <string>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib")
using sock_t = SOCKET;
static void closeSock(sock_t s) { closesocket(s); }
static bool goodSock(sock_t s) { return s != INVALID_SOCKET; }
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
using sock_t = int;
static void closeSock(sock_t s) { close(s); }
static bool goodSock(sock_t s) { return s >= 0; }
#endif
namespace core {
static std::string esc(const std::string& s) { std::string o; for(char c:s){ if(c=='\"')o+="\\\""; else if(c=='\\')o+="\\\\"; else if(c=='\n')o+="\\n"; else if(c=='\r')o+="\\r"; else if(c=='\t')o+="\\t"; else o+=c; } return o; }
static std::string urlDecode(const std::string& s){std::string o;for(size_t i=0;i<s.size();++i){if(s[i]=='%'&&i+2<s.size()){auto h=[](char c){if(c>='0'&&c<='9')return c-'0';if(c>='A'&&c<='F')return c-'A'+10;if(c>='a'&&c<='f')return c-'a'+10;return -1;};int a=h(s[i+1]),b=h(s[i+2]);if(a>=0&&b>=0){o.push_back((char)((a<<4)|b));i+=2;continue;}}o.push_back(s[i]=='+'?' ':s[i]);}return o;}
static std::string jsonField(const std::string& j,const std::string& k){std::string m="\""+k+"\":\"";auto p=j.find(m);if(p==std::string::npos)return{};p+=m.size();std::string o;bool sl=false;for(;p<j.size();++p){char c=j[p];if(sl){if(c=='n')o+='\n';else if(c=='r')o+='\r';else if(c=='t')o+='\t';else o+=c;sl=false;continue;}if(c=='\\'){sl=true;continue;}if(c=='\"')break;o+=c;}return o;}
static bool jsonBool(const std::string&j,const std::string&k){std::string m="\""+k+"\":";auto p=j.find(m);if(p==std::string::npos)return false;p+=m.size();return j.compare(p,4,"true")==0;}
static std::string response(int code,const std::string&type,const std::string&body){const char* t=code==200?"OK":code==400?"Bad Request":code==403?"Forbidden":code==404?"Not Found":"Error";return "HTTP/1.1 "+std::to_string(code)+" "+t+"\r\nContent-Type: "+type+"\r\nContent-Length: "+std::to_string(body.size())+"\r\nAccess-Control-Allow-Origin: http://127.0.0.1:47821\r\nAccess-Control-Allow-Headers: Content-Type\r\nAccess-Control-Allow-Methods: GET, POST, OPTIONS\r\nConnection: close\r\n\r\n"+body;}
static void sendAll(sock_t s,const std::string&d){size_t n=0;while(n<d.size()){int w=send(s,d.data()+n,(int)std::min<std::size_t>(d.size()-n,1<<20),0);if(w<=0)break;n+=(size_t)w;}}
static void handle(sock_t s,CoreApp&app,const std::string&web){std::string req;char b[8192];for(int i=0;i<64;i++){int n=recv(s,b,sizeof(b),0);if(n<=0)break;req.append(b,n);if(req.find("\r\n\r\n")!=std::string::npos&&req.size()>65536)break;if(req.size()>131072)break;}auto e=req.find("\r\n");if(e==std::string::npos){sendAll(s,response(400,"text/plain","bad request"));closeSock(s);return;}std::istringstream first(req.substr(0,e));std::string method,path,ver;first>>method>>path>>ver;auto h=req.find("\r\n\r\n");std::string body=h==std::string::npos?"":req.substr(h+4);
if(method=="OPTIONS"){sendAll(s,response(200,"text/plain","ok"));closeSock(s);return;}
if(path=="/api/health"){sendAll(s,response(200,"application/json","{\"ok\":true,\"service\":\"core-ai\"}"));closeSock(s);return;}
if(path=="/status"||path=="/api/status"){sendAll(s,response(200,"application/json",app.status()));closeSock(s);return;}
if(path=="/capabilities"||path=="/api/capabilities"){sendAll(s,response(200,"application/json","{\"capabilities\":\""+esc(app.capabilities())+"\"}"));closeSock(s);return;}
if((path=="/chat"||path=="/api/chat")&&method=="POST"){auto mode=jsonField(body,"mode");auto prompt=jsonField(body,"prompt");if(prompt.empty())prompt=jsonField(body,"message");const bool think=jsonBool(body,"think");const auto selectedModel=jsonField(body,"model");if(mode.empty()&&(think||selectedModel=="think"))mode="think";std::string out;if(prompt.empty())out="ERROR: prompt required";else if(mode=="think")out=app.think(prompt);else if(mode=="code")out=app.code(prompt);else if(mode=="agent")out=app.agent(prompt);else out=app.chat(prompt);const bool ok=out.rfind("MODEL ERROR:",0)!=0&&out.rfind("ERROR:",0)!=0;sendAll(s,response(ok?200:400,"application/json","{\"text\":\""+esc(out)+"\",\"content\":\""+esc(out)+"\"}"));closeSock(s);return;}
if(path.rfind("/remember",0)==0&&method=="POST"){auto text=jsonField(body,"text");auto scope=jsonField(body,"scope");bool ok=!text.empty()&&app.remember(text,scope.empty()?"personal":scope);sendAll(s,response(ok?200:403,"application/json",ok?"{\"ok\":true}":"{\"ok\":false}"));closeSock(s);return;}
if(path.rfind("/project-create",0)==0&&method=="POST"){auto name=jsonField(body,"name");app.grant("file.write");auto result=app.projectCreate(name);bool ok=result.rfind("PROJECT CREATED",0)==0;sendAll(s,response(ok?200:400,"application/json","{\"text\":\""+esc(result)+"\"}"));closeSock(s);return;}
if(method=="GET"&&path.rfind("/memory-search",0)==0){auto qpos=path.find("?q=");auto q=qpos==std::string::npos?std::string():urlDecode(path.substr(qpos+3));auto mem=app.memorySearch(q);sendAll(s,response(200,"application/json","{\"text\":\""+esc(mem)+"\"}"));closeSock(s);return;}
if(method=="GET"&&path=="/projects"){auto list=app.projectList();sendAll(s,response(200,"application/json","{\"text\":\""+esc(list)+"\"}"));closeSock(s);return;}
if(method=="GET"){std::string file=path=="/"?"/index.html":path;if(file.find("..")!=std::string::npos){sendAll(s,response(403,"text/plain","forbidden"));closeSock(s);return;}auto data=tools::readFile((std::filesystem::path(web)/file.substr(1)).string());if(data.empty())sendAll(s,response(404,"text/plain","not found"));else{std::string type="text/plain";if(file.ends_with(".html"))type="text/html; charset=utf-8";else if(file.ends_with(".css"))type="text/css; charset=utf-8";else if(file.ends_with(".js"))type="application/javascript; charset=utf-8";sendAll(s,response(200,type,data));}closeSock(s);return;}
sendAll(s,response(405,"text/plain","method not allowed"));closeSock(s);}
bool LocalApiServer::start(CoreApp&app,std::uint16_t port,std::string root){app_=&app;port_=port;webroot_=std::move(root);running_=true;return true;}void LocalApiServer::stop(){running_=false;}void LocalApiServer::run(){if(!app_)return;
#ifdef _WIN32
WSADATA w{};if(WSAStartup(MAKEWORD(2,2),&w)!=0)return;
#endif
sock_t s=socket(AF_INET,SOCK_STREAM,0);if(!goodSock(s))return;int opt=1;setsockopt(s,SOL_SOCKET,SO_REUSEADDR,(char*)&opt,sizeof(opt));sockaddr_in addr{};addr.sin_family=AF_INET;addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);addr.sin_port=htons(port_);if(bind(s,(sockaddr*)&addr,sizeof(addr))<0){closeSock(s);return;}if(listen(s,8)<0){closeSock(s);return;}while(running_){sockaddr_in c{};
#ifdef _WIN32
int len=sizeof(c);
#else
socklen_t len=sizeof(c);
#endif
sock_t cs=accept(s,(sockaddr*)&c,&len);if(!goodSock(cs))continue;handle(cs,*app_,webroot_);}closeSock(s);
#ifdef _WIN32
WSACleanup();
#endif
}
}
