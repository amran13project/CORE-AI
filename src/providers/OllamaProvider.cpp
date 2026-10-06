#include "providers/OllamaProvider.h"
#include <chrono>
#include <sstream>
#include <thread>
#include <cstring>
#include <algorithm>
#include <cstdlib>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif
namespace coreai::providers {
static std::string esc(const std::string&s){std::string o;for(char c:s){if(c=='"')o+="\\\"";else if(c=='\\')o+="\\\\";else if(c=='\n')o+="\\n";else if(c=='\r')o+="\\r";else o+=c;}return o;}
static std::string jsonField(const std::string&j,const std::string&k){
    const std::string needle="\""+k+"\"";auto p=j.find(needle);if(p==std::string::npos)return{};p+=needle.size();
    while(p<j.size()&&(j[p]==' '||j[p]=='\t'||j[p]=='\r'||j[p]=='\n'))++p;if(p>=j.size()||j[p]!=':')return{};++p;
    while(p<j.size()&&(j[p]==' '||j[p]=='\t'||j[p]=='\r'||j[p]=='\n'))++p;if(p>=j.size()||j[p]!='\"')return{};++p;
    std::string o;bool escaped=false;for(;p<j.size();++p){char c=j[p];if(escaped){if(c=='n')o+='\n';else if(c=='r')o+='\r';else if(c=='t')o+='\t';else o+=c;escaped=false;continue;}if(c=='\\'){escaped=true;continue;}if(c=='\"')break;o+=c;}return o;
}
static Result<std::string> httpPost(const std::string&url,const std::string&body,const std::string&method){
    // Ollama's default endpoint is local HTTP. Use a direct TCP transport first so
    // the provider does not inherit browser/proxy behavior. HTTPS remains outside
    // this local adapter's scope.
    if(url.rfind("http://",0)!=0)
        return Result<std::string>::failure(error(ErrorCode::ProviderUnavailable,"Only http:// Ollama endpoints are supported by the local adapter","provider","http",true,false));
    std::string u=url.substr(7);
    auto slash=u.find('/');
    std::string authority=slash==std::string::npos?u:u.substr(0,slash);
    std::string path=slash==std::string::npos?"/":u.substr(slash);
    std::string host=authority;
    int port=11434;
    if(!host.empty() && host.front()=='['){
        auto rb=host.find(']');
        if(rb==std::string::npos) return Result<std::string>::failure(error(ErrorCode::ProviderUnavailable,"invalid Ollama IPv6 host","provider","http"));
        std::string remainder=host.substr(rb+1);
        host=host.substr(1,rb-1);
        if(!remainder.empty() && remainder.front()==':') port=std::stoi(remainder.substr(1));
    }else{
        auto colon=host.rfind(':');
        if(colon!=std::string::npos && host.find(':')==colon){ port=std::stoi(host.substr(colon+1)); host=host.substr(0,colon); }
    }
    if(host.empty() || port<1 || port>65535)
        return Result<std::string>::failure(error(ErrorCode::ProviderUnavailable,"invalid Ollama host/port","provider","http"));
#ifdef _WIN32
    WSADATA wsa{};
    if(WSAStartup(MAKEWORD(2,2),&wsa)!=0)
        return Result<std::string>::failure(error(ErrorCode::ProviderUnavailable,"Winsock initialization failed","provider","http",true,true));
    SOCKET s=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(s==INVALID_SOCKET){WSACleanup();return Result<std::string>::failure(error(ErrorCode::ProviderUnavailable,"socket creation failed","provider","http",true,true));}
    DWORD timeout=120000;
    if(const char* env=std::getenv("CORE_OLLAMA_TIMEOUT_MS")){
        try{
            const unsigned long v=std::stoul(env);
            if(v>=1000 && v<=600000) timeout=static_cast<DWORD>(v);
        }catch(...){}
    }
    setsockopt(s,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout,sizeof(timeout));
    setsockopt(s,SOL_SOCKET,SO_SNDTIMEO,(const char*)&timeout,sizeof(timeout));
    sockaddr_in a{};a.sin_family=AF_INET;a.sin_port=htons((u_short)port);
    if(inet_pton(AF_INET,host.c_str(),&a.sin_addr)!=1){closesocket(s);WSACleanup();return Result<std::string>::failure(error(ErrorCode::ProviderUnavailable,"only numeric IPv4 local Ollama host is supported","provider","http"));}
    if(connect(s,(sockaddr*)&a,sizeof(a))==SOCKET_ERROR){int e=WSAGetLastError();closesocket(s);WSACleanup();return Result<std::string>::failure(error(ErrorCode::ProviderUnavailable,"connect failed (Winsock "+std::to_string(e)+")","provider","http",true,true));}
    auto sendAll=[&](const std::string&d)->bool{size_t off=0;while(off<d.size()){int n=send(s,d.data()+off,(int)std::min<size_t>(d.size()-off,1<<20),0);if(n<=0)return false;off+=n;}return true;};
    std::ostringstream req;req<<(method=="GET"?"GET":"POST")<<' '<<path<<" HTTP/1.1\r\nHost: "<<host<<":"<<port<<"\r\nConnection: close\r\nAccept: application/json\r\nContent-Type: application/json\r\nContent-Length: "<<body.size()<<"\r\n\r\n"<<body;
    const auto raw=req.str();
    if(!sendAll(raw)){closesocket(s);WSACleanup();return Result<std::string>::failure(error(ErrorCode::ProviderUnavailable,"send failed","provider","http",true,true));}
    std::string resp;char buf[8192];for(;;){int n=recv(s,buf,(int)sizeof(buf),0);if(n==0)break;if(n==SOCKET_ERROR){int e=WSAGetLastError();closesocket(s);WSACleanup();return Result<std::string>::failure(error(ErrorCode::ProviderUnavailable,"receive failed (Winsock "+std::to_string(e)+")","provider","http",true,true));}resp.append(buf,n);}closesocket(s);WSACleanup();
#else
    int s=socket(AF_INET,SOCK_STREAM,0);if(s<0)return Result<std::string>::failure(error(ErrorCode::ProviderUnavailable,"socket creation failed","provider","http",true,true));
    int timeoutMs=120000;
    if(const char* env=std::getenv("CORE_OLLAMA_TIMEOUT_MS")){
        try{
            const int v=std::stoi(env);
            if(v>=1000 && v<=600000) timeoutMs=v;
        }catch(...){}
    }
    timeval tv{timeoutMs/1000,(timeoutMs%1000)*1000};
    setsockopt(s,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv));
    setsockopt(s,SOL_SOCKET,SO_SNDTIMEO,&tv,sizeof(tv));
    sockaddr_in a{};a.sin_family=AF_INET;a.sin_port=htons((uint16_t)port);if(inet_pton(AF_INET,host.c_str(),&a.sin_addr)<=0){close(s);return Result<std::string>::failure(error(ErrorCode::ProviderUnavailable,"only numeric IPv4 local Ollama host is supported","provider","http"));}
    if(connect(s,(sockaddr*)&a,sizeof(a))<0){close(s);return Result<std::string>::failure(error(ErrorCode::ProviderUnavailable,"connect failed","provider","http",true,true));}
    auto sendAll=[&](const std::string&d)->bool{size_t off=0;while(off<d.size()){ssize_t n=send(s,d.data()+off,d.size()-off,0);if(n<=0)return false;off+=static_cast<size_t>(n);}return true;};
    std::ostringstream req;req<<(method=="GET"?"GET":"POST")<<' '<<path<<" HTTP/1.1\r\nHost: "<<host<<":"<<port<<"\r\nConnection: close\r\nAccept: application/json\r\nContent-Type: application/json\r\nContent-Length: "<<body.size()<<"\r\n\r\n"<<body;const auto raw=req.str();
    if(!sendAll(raw)){close(s);return Result<std::string>::failure(error(ErrorCode::ProviderUnavailable,"send failed","provider","http",true,true));}
    std::string resp;char buf[8192];for(;;){ssize_t n=recv(s,buf,sizeof(buf),0);if(n<=0)break;resp.append(buf,n);}close(s);
#endif
    auto lineEnd=resp.find("\r\n");auto sep=resp.find("\r\n\r\n");
    if(lineEnd==std::string::npos||sep==std::string::npos)return Result<std::string>::failure(error(ErrorCode::ProviderUnavailable,"invalid HTTP response","provider","http",true,true));
    auto statusLine=resp.substr(0,lineEnd);int status=0;{std::istringstream ss(statusLine);std::string http;ss>>http>>status;}
    if(status<200||status>=300)return Result<std::string>::failure(error(ErrorCode::ProviderUnavailable,"HTTP status "+std::to_string(status),"provider","http",true,status==408||status==429||status>=500));
    return Result<std::string>::success(resp.substr(sep+4));
}
OllamaProvider::OllamaProvider(std::string url,std::string model):url_(std::move(url)),model_(std::move(model)){}
bool OllamaProvider::reachable()const{return httpPost(url_+"/api/tags",{},"GET").ok();}
Result<std::vector<Model>> OllamaProvider::discover(){auto r=httpPost(url_+"/api/tags",{},"GET");if(!r.ok())return Result<std::vector<Model>>::failure(r.error());std::vector<Model> out;auto j=r.value();size_t p=0;while((p=j.find("\"name\":\"",p))!=std::string::npos){p+=8;auto e=j.find('"',p);if(e==std::string::npos)break;auto n=j.substr(p,e-p);out.push_back({"ollama",n,n,false,false,true});p=e+1;}return Result<std::vector<Model>>::success(std::move(out));}
Result<GenerationResponse> OllamaProvider::generate(const GenerationRequest&q){auto start=std::chrono::steady_clock::now();std::ostringstream b;b<<"{\"model\":\""<<esc(q.model.empty()?model_:q.model)<<"\",\"messages\":[";if(!q.system.empty())b<<"{\"role\":\"system\",\"content\":\""<<esc(q.system)<<"\"},";b<<"{\"role\":\"user\",\"content\":\""<<esc(q.prompt)<<"\"}],\"stream\":false}";auto r=httpPost(url_+"/api/chat",b.str(),"POST");if(!r.ok())return Result<GenerationResponse>::failure(r.error());auto text=jsonField(r.value(),"content");if(text.empty())return Result<GenerationResponse>::failure(error(ErrorCode::ModelUnavailable,"Ollama returned no content","provider","generate"));auto ms=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-start).count();return Result<GenerationResponse>::success({text,"ollama",q.model.empty()?model_:q.model,ms});}
}
