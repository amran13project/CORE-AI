#include "services/LocalApiServer.h"
#include "services/CoreApp.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>
#include <cctype>
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
static const char* errorCodeName(ErrorCode c){
    switch(c){
        case ErrorCode::InvalidArgument:return "COREAI_INVALID_ARGUMENT";
        case ErrorCode::ConfigInvalid:return "COREAI_CONFIG_INVALID";
        case ErrorCode::StorageOpenFailed:return "COREAI_STORAGE_OPEN_FAILED";
        case ErrorCode::StorageUnavailable:return "COREAI_STORAGE_UNAVAILABLE";
        case ErrorCode::StorageCorrupt:return "COREAI_STORAGE_CORRUPT";
        case ErrorCode::PermissionDenied:return "COREAI_PERMISSION_DENIED";
        case ErrorCode::ProviderUnavailable:return "COREAI_PROVIDER_UNAVAILABLE";
        case ErrorCode::ModelUnavailable:return "COREAI_MODEL_UNAVAILABLE";
        case ErrorCode::NetworkTimeout:return "COREAI_NETWORK_TIMEOUT";
        case ErrorCode::ParseError:return "COREAI_PARSE_ERROR";
        case ErrorCode::NotImplemented:return "COREAI_NOT_IMPLEMENTED";
        case ErrorCode::NotVerified:return "COREAI_NOT_VERIFIED";
        default:return "COREAI_INTERNAL";
    }
}
static std::string urlDecode(const std::string& s){std::string o;for(size_t i=0;i<s.size();++i){if(s[i]=='%'&&i+2<s.size()){auto h=[](char c)->int{if(c>='0'&&c<='9')return c-'0';if(c>='A'&&c<='F')return c-'A'+10;if(c>='a'&&c<='f')return c-'a'+10;return -1;};int a=h(s[i+1]),b=h(s[i+2]);if(a>=0&&b>=0){o.push_back(char((a<<4)|b));i+=2;continue;}}o.push_back(s[i]=='+'?' ':s[i]);}return o;}
static std::string field(const std::string&j,const std::string&k){
    const std::string needle="\""+k+"\""; auto p=j.find(needle); if(p==std::string::npos)return{};
    p+=needle.size(); p=j.find(':',p); if(p==std::string::npos)return{}; ++p;
    while(p<j.size() && (j[p]==' '||j[p]=='\t'||j[p]=='\r'||j[p]=='\n'))++p;
    if(p>=j.size()||j[p]!='\"')return{}; ++p; std::string out; bool escp=false;
    for(;p<j.size();++p){char c=j[p];if(escp){if(c=='n')out+='\n';else if(c=='r')out+='\r';else if(c=='t')out+='\t';else out+=c;escp=false;continue;}if(c=='\\'){escp=true;continue;}if(c=='\"')break;out+=c;}return out;
}
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
void LocalApiServer::handle(int s){
    std::string req; char b[8192]; size_t headerEnd=std::string::npos; size_t contentLength=0;
    for(int i=0;i<64;i++){
        int n=recv(s,b,sizeof(b),0); if(n<=0) break; req.append(b,n);
        if(headerEnd==std::string::npos){
            headerEnd=req.find("\r\n\r\n");
            if(headerEnd!=std::string::npos){
                auto hs=req.substr(0,headerEnd); auto lower=hs; for(char& ch:lower) ch=(char)std::tolower((unsigned char)ch);
                auto pos=lower.find("content-length:");
                if(pos!=std::string::npos){pos+=15; while(pos<lower.size()&&(lower[pos]==' '||lower[pos]=='\t'))++pos; try{contentLength=std::stoull(lower.substr(pos));}catch(...){contentLength=0;}}
            }
        }
        if(headerEnd!=std::string::npos && req.size()>=headerEnd+4+contentLength) break;
        if(req.size()>2621440) break;
    }
    auto e=req.find("\r\n"); if(e==std::string::npos){auto x=resp(400,"text/plain","bad request");send(s,x.data(),(int)x.size(),0);close_sock(s);return;}
    std::istringstream first(req.substr(0,e)); std::string method,path,ver; first>>method>>path>>ver;
    auto hh=req.find("\r\n\r\n"); std::string body=hh==std::string::npos?"":req.substr(hh+4,contentLength?contentLength:std::string::npos);
    auto qpos=path.find('?'); std::string route=qpos==std::string::npos?path:path.substr(0,qpos);
    std::string out; int code=200; std::string type="application/json";
    auto fail=[&](int c,const std::string& m){code=c;out="{\"error\":\""+esc(m)+"\"}";};
    if(route=="/health") out="{\"status\":\"OK\"}";
    else if(route=="/ready"){auto r=app_.status();if(r.ok())out=r.value();else fail(500,r.error().message);}
    else if(route=="/version") out="{\"version\":\"0.3.2\",\"api\":\"v1\"}";
    else if(route=="/status"){auto r=app_.status();if(r.ok())out=r.value();else fail(500,r.error().message);}
    else if(route=="/capabilities") out=app_.capabilities();
    else if(route=="/doctor") out=app_.doctor();
    else if(route=="/models"){auto r=app_.models();if(r.ok())out=r.value();else fail(503,r.error().message);}
    else if(route=="/chat"&&method=="POST"){auto r=app_.chat(field(body,"prompt"),field(body,"mode").empty()?"chat":field(body,"mode"));if(r.ok())out="{\"text\":\""+esc(r.value())+"\"}";else{code=r.error().code==ErrorCode::ProviderUnavailable?503:400;out="{\"error\":\""+esc(r.error().message)+"\",\"error_code\":\""+errorCodeName(r.error().code)+"\",\"retryable\":"+(r.error().retryable?"true":"false")+"}";}}
    else if(route=="/conversations/new"&&method=="POST"){auto r=app_.newChat();if(r.ok())out="{\"id\":\""+esc(r.value())+"\"}";else fail(500,r.error().message);}
    else if(route=="/conversations/reset"&&method=="POST"){auto r=app_.resetChat();if(r.ok())out="{\"id\":\""+esc(r.value())+"\"}";else fail(500,r.error().message);}
    else if(route=="/conversations/recent"&&method=="GET"){auto r=app_.recent();if(r.ok())out=r.value();else fail(500,r.error().message);}
    else if(route=="/memory"&&method=="POST"){auto r=app_.remember(field(body,"text"),field(body,"scope").empty()?"personal":field(body,"scope"));if(r.ok())out="{\"id\":\""+esc(r.value())+"\"}";else fail(400,r.error().message);}
    else if(route=="/memory/search"&&method=="GET"){auto q=qpos==std::string::npos?std::string():path.substr(qpos+1);auto qp=q.find("q=");auto query=qp==std::string::npos?std::string():urlDecode(q.substr(qp+2));auto r=app_.memorySearch(query);if(r.ok())out=r.value();else fail(500,r.error().message);}
    else if(route=="/projects"&&method=="GET"){auto r=app_.projectList();if(r.ok())out=r.value();else fail(500,r.error().message);}
    else if(route=="/projects"&&method=="POST"){auto r=app_.projectCreate(field(body,"name"));if(r.ok())out="{\"id\":\""+esc(r.value())+"\"}";else fail(400,r.error().message);}
    else if(route=="/library"&&method=="GET"){auto r=app_.libraryList();if(r.ok())out=r.value();else fail(500,r.error().message);}
    else if(route=="/library"&&method=="POST"){auto r=app_.libraryAdd(field(body,"name"),field(body,"content"));if(r.ok())out="{\"id\":\""+esc(r.value())+"\"}";else fail(400,r.error().message);}
    else if(route=="/files/write"&&method=="POST"){auto r=app_.fileWrite(field(body,"path"),field(body,"content"));if(r.ok())out="{\"path\":\""+esc(r.value())+"\"}";else fail(400,r.error().message);}
    else if(route=="/files/read"&&method=="GET"){auto q=qpos==std::string::npos?std::string():path.substr(qpos+1);auto pp=q.find("path=");auto rr=pp==std::string::npos?std::string():urlDecode(q.substr(pp+5));auto r=app_.fileRead(rr);if(r.ok())out="{\"content\":\""+esc(r.value())+"\"}";else fail(400,r.error().message);}
    else if(route=="/documents/inspect"&&method=="GET"){auto q=qpos==std::string::npos?std::string():path.substr(qpos+1);auto pp=q.find("path=");auto rr=pp==std::string::npos?std::string():urlDecode(q.substr(pp+5));auto r=app_.documentInspect(rr);if(r.ok())out=r.value();else fail(400,r.error().message);}
    else if(route=="/tasks"&&method=="GET"){auto r=app_.taskList();if(r.ok())out=r.value();else fail(500,r.error().message);}
    else if(route=="/tasks"&&method=="POST"){auto r=app_.taskCreate(field(body,"name"));if(r.ok())out="{\"id\":\""+esc(r.value())+"\"}";else fail(400,r.error().message);}
    else if(route=="/workflows"&&method=="GET"){auto r=app_.workflowList();if(r.ok())out=r.value();else fail(500,r.error().message);}
    else if(route=="/workflows"&&method=="POST"){auto r=app_.workflowCreate(field(body,"name"),field(body,"definition"));if(r.ok())out="{\"id\":\""+esc(r.value())+"\"}";else fail(400,r.error().message);}
    else if(route=="/workflows/run"&&method=="POST"){auto r=app_.workflowRun(field(body,"id"));if(r.ok())out=r.value();else fail(400,r.error().message);}
    else if(route=="/scheduled"&&method=="GET"){auto r=app_.scheduleList();if(r.ok())out=r.value();else fail(500,r.error().message);}
    else if(route=="/scheduled"&&method=="POST"){long long d=0;try{d=std::stoll(field(body,"delay_seconds"));}catch(...){d=0;}auto r=app_.scheduleCreate(field(body,"name"),d);if(r.ok())out="{\"id\":\""+esc(r.value())+"\"}";else fail(400,r.error().message);}
    else if(route=="/scheduled/run"&&method=="POST"){auto r=app_.scheduleRun(field(body,"id"));if(r.ok())out=r.value();else fail(400,r.error().message);}
    else if(route=="/agents"&&method=="POST"){auto r=app_.agentRun(field(body,"request"));if(r.ok())out=r.value();else fail(400,r.error().message);}
    else if(route=="/multi-agent"&&method=="POST"){auto r=app_.multiAgentRun(field(body,"request"));if(r.ok())out=r.value();else fail(400,r.error().message);}
    else if(route=="/plugins"&&method=="GET"){auto r=app_.pluginDiscover();if(r.ok())out=r.value();else fail(500,r.error().message);}
    else if(route=="/git/status"&&method=="GET"){auto q=qpos==std::string::npos?std::string():path.substr(qpos+1);auto pp=q.find("path=");auto rr=pp==std::string::npos?std::filesystem::current_path():std::filesystem::path(urlDecode(q.substr(pp+5)));auto r=app_.gitStatus(rr);if(r.ok())out="{\"status\":\""+esc(r.value())+"\"}";else fail(400,r.error().message);}
    else if(route=="/research"&&method=="GET"){auto q=qpos==std::string::npos?std::string():path.substr(qpos+1);auto qp=q.find("q=");auto query=qp==std::string::npos?std::string():urlDecode(q.substr(qp+2));auto r=app_.researchLocal(query);if(r.ok())out=r.value();else fail(500,r.error().message);}
    else if(route=="/images"&&method=="POST"){auto r=app_.imageCreate(field(body,"name").empty()?"image":field(body,"name"),field(body,"prompt"));if(r.ok())out="{\"artifact\":\""+esc(r.value())+"\"}";else fail(400,r.error().message);}
    else if(route=="/maps/search"&&method=="GET"){auto q=qpos==std::string::npos?std::string():path.substr(qpos+1);auto qp=q.find("q=");auto query=qp==std::string::npos?std::string():urlDecode(q.substr(qp+2));auto r=app_.mapSearch(query);if(r.ok())out=r.value();else fail(500,r.error().message);}
    else if(route=="/maps/directions"&&method=="GET"){auto q=qpos==std::string::npos?std::string():path.substr(qpos+1);auto a=q.find("from="), bq=q.find("&to=");auto from=a==std::string::npos?std::string():urlDecode(q.substr(a+5,(bq==std::string::npos?q.size():bq)-a-5));auto to=bq==std::string::npos?std::string():urlDecode(q.substr(bq+4));auto r=app_.directions(from,to);if(r.ok())out=r.value();else fail(500,r.error().message);}
    else if(route=="/voice"||route=="/vision"){code=501;out="{\"status\":\"NOT_IMPLEMENTED\",\"reason\":\"No verified local backend in this environment\"}";}
    else if(route=="/multimodal"&&method=="GET")out="{\"status\":\"READY\",\"modes\":[\"text\",\"file\",\"document\",\"image-artifact\"]}";
    else if(method=="GET"){
        std::string f=route=="/"?"/index.html":route;
        if(f.find("..")!=std::string::npos){code=403;type="text/plain";out="forbidden";}
        else{auto p=web_root_/f.substr(1);std::error_code ec;auto canon=std::filesystem::weakly_canonical(p,ec);auto base=std::filesystem::weakly_canonical(web_root_);auto rel=std::filesystem::relative(canon,base,ec);if(ec||(!rel.empty()&&*rel.begin()=="..")){code=403;type="text/plain";out="forbidden";}else{std::ifstream in(canon,std::ios::binary);if(!in){code=404;type="text/plain";out="not found";}else{std::ostringstream ss;ss<<in.rdbuf();out=ss.str();if(canon.extension()==".html")type="text/html";else if(canon.extension()==".css")type="text/css";else if(canon.extension()==".js")type="application/javascript";else type="application/octet-stream";}}}
    } else fail(404,"not found");
    auto x=resp(code,type,out);send(s,x.data(),(int)x.size(),0);close_sock(s);
}
}
