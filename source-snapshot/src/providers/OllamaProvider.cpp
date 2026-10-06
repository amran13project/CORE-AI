#include "providers/OllamaProvider.h"
#include <chrono>
#include <sstream>
#include <thread>
#include <cstring>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winhttp.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif
namespace coreai::providers {
static std::string esc(const std::string&s){std::string o;for(char c:s){if(c=='"')o+="\\\"";else if(c=='\\')o+="\\\\";else if(c=='\n')o+="\\n";else if(c=='\r')o+="\\r";else o+=c;}return o;}
static std::string jsonField(const std::string&j,const std::string&k){auto p=j.find("\""+k+"\":\"");if(p==std::string::npos)return{};p+=k.size()+4;std::string o;bool s=false;for(;p<j.size();++p){char c=j[p];if(s){if(c=='n')o+='\n';else if(c=='r')o+='\r';else if(c=='t')o+='\t';else o+=c;s=false;continue;}if(c=='\\'){s=true;continue;}if(c=='"')break;o+=c;}return o;}
static Result<std::string> httpPost(const std::string&url,const std::string&body,const std::string&method){
#ifdef _WIN32
    std::string u=url; bool https=u.rfind("https://",0)==0; if(https)u.erase(0,8); else if(u.rfind("http://",0)==0)u.erase(0,7);auto slash=u.find('/');std::string host=slash==std::string::npos?u:u.substr(0,slash);std::string path=slash==std::string::npos?"/":u.substr(slash);
    int wn=MultiByteToWideChar(CP_UTF8,0,host.c_str(),-1,nullptr,0);std::wstring wh(wn,0);MultiByteToWideChar(CP_UTF8,0,host.c_str(),-1,wh.data(),wn);
    int pn=MultiByteToWideChar(CP_UTF8,0,path.c_str(),-1,nullptr,0);std::wstring wp(pn,0);MultiByteToWideChar(CP_UTF8,0,path.c_str(),-1,wp.data(),pn);
    HINTERNET s=WinHttpOpen(L"CORE-AI/0.3",WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,nullptr,nullptr,0);if(!s)return Result<std::string>::failure(error(ErrorCode::ProviderUnavailable,"WinHTTP unavailable","provider","http"));
    HINTERNET c=WinHttpConnect(s,wh.c_str(),https?INTERNET_DEFAULT_HTTPS_PORT:INTERNET_DEFAULT_HTTP_PORT,0);if(!c){WinHttpCloseHandle(s);return Result<std::string>::failure(error(ErrorCode::ProviderUnavailable,"connect failed","provider","http",true,true));}
    HINTERNET r=WinHttpOpenRequest(c,method=="GET"?L"GET":L"POST",wp.c_str(),nullptr,WINHTTP_NO_REFERER,WINHTTP_DEFAULT_ACCEPT_TYPES,https?WINHTTP_FLAG_SECURE:0);if(!r){WinHttpCloseHandle(c);WinHttpCloseHandle(s);return Result<std::string>::failure(error(ErrorCode::ProviderUnavailable,"request failed","provider","http",true,true));}
    LPCWSTR h=L"Content-Type: application/json\r\nAccept: application/json\r\n";BOOL ok=method=="GET"?WinHttpSendRequest(r,h,(DWORD)-1L,nullptr,0,0,0):WinHttpSendRequest(r,h,(DWORD)-1L,(LPVOID)body.data(),(DWORD)body.size(),(DWORD)body.size(),0);if(ok)ok=WinHttpReceiveResponse(r,nullptr);std::string out;if(ok){DWORD n=0;while(WinHttpQueryDataAvailable(r,&n)&&n){std::string b(n,'\0');DWORD g=0;WinHttpReadData(r,b.data(),n,&g);out.append(b.data(),g);}}WinHttpCloseHandle(r);WinHttpCloseHandle(c);WinHttpCloseHandle(s);if(!ok)return Result<std::string>::failure(error(ErrorCode::ProviderUnavailable,"HTTP request failed","provider","http",true,true));return Result<std::string>::success(out);
#else
    if(url.rfind("http://",0)!=0)return Result<std::string>::failure(error(ErrorCode::ProviderUnavailable,"Only local HTTP is implemented in this build","provider","http",true));
    std::string u=url.substr(7);auto slash=u.find('/');std::string host=slash==std::string::npos?u:u.substr(0,slash);std::string path=slash==std::string::npos?"/":u.substr(slash);auto colon=host.find(':');int port=11434;if(colon!=std::string::npos){port=std::stoi(host.substr(colon+1));host=host.substr(0,colon);}int s=socket(AF_INET,SOCK_STREAM,0);if(s<0)return Result<std::string>::failure(error(ErrorCode::ProviderUnavailable,"socket failed","provider","http"));sockaddr_in a{};a.sin_family=AF_INET;a.sin_port=htons((uint16_t)port);if(inet_pton(AF_INET,host.c_str(),&a.sin_addr)<=0){close(s);return Result<std::string>::failure(error(ErrorCode::ProviderUnavailable,"only numeric local address supported","provider","http"));}if(connect(s,(sockaddr*)&a,sizeof(a))<0){close(s);return Result<std::string>::failure(error(ErrorCode::ProviderUnavailable,"connect failed","provider","http",true,true));}std::ostringstream req;req<<(method=="GET"?"GET ":"POST ")<<path<<" HTTP/1.1\r\nHost: "<<host<<"\r\nConnection: close\r\nContent-Type: application/json\r\nContent-Length: "<<body.size()<<"\r\n\r\n"<<body;auto raw=req.str();send(s,raw.data(),raw.size(),0);std::string resp;char buf[4096];for(;;){int n=recv(s,buf,sizeof(buf),0);if(n<=0)break;resp.append(buf,n);}close(s);auto h=resp.find("\r\n\r\n");if(h==std::string::npos)return Result<std::string>::failure(error(ErrorCode::ProviderUnavailable,"invalid HTTP response","provider","http"));return Result<std::string>::success(resp.substr(h+4));
#endif
}
OllamaProvider::OllamaProvider(std::string url,std::string model):url_(std::move(url)),model_(std::move(model)){}
bool OllamaProvider::reachable()const{return httpPost(url_+"/api/tags",{},"GET").ok();}
Result<std::vector<Model>> OllamaProvider::discover(){auto r=httpPost(url_+"/api/tags",{},"GET");if(!r.ok())return Result<std::vector<Model>>::failure(r.error());std::vector<Model> out;auto j=r.value();size_t p=0;while((p=j.find("\"name\":\"",p))!=std::string::npos){p+=8;auto e=j.find('"',p);if(e==std::string::npos)break;auto n=j.substr(p,e-p);out.push_back({"ollama",n,n,false,false,true});p=e+1;}return Result<std::vector<Model>>::success(std::move(out));}
Result<GenerationResponse> OllamaProvider::generate(const GenerationRequest&q){auto start=std::chrono::steady_clock::now();std::ostringstream b;b<<"{\"model\":\""<<esc(q.model.empty()?model_:q.model)<<"\",\"messages\":[";if(!q.system.empty())b<<"{\"role\":\"system\",\"content\":\""<<esc(q.system)<<"\"},";b<<"{\"role\":\"user\",\"content\":\""<<esc(q.prompt)<<"\"}],\"stream\":false}";auto r=httpPost(url_+"/api/chat",b.str(),"POST");if(!r.ok())return Result<GenerationResponse>::failure(r.error());auto text=jsonField(r.value(),"content");if(text.empty())return Result<GenerationResponse>::failure(error(ErrorCode::ModelUnavailable,"Ollama returned no content","provider","generate"));auto ms=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-start).count();return Result<GenerationResponse>::success({text,"ollama",q.model.empty()?model_:q.model,ms});}
}
