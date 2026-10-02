#include "ai/OllamaProvider.h"
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <string>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winhttp.h>
#endif
namespace core::ai {
static std::string E(const char*k){const char*p=std::getenv(k);return p?p:"";}
static std::string esc(const std::string&s){std::string o;for(char c:s){if(c=='"')o+="\\\"";else if(c=='\\')o+="\\\\";else if(c=='\n')o+="\\n";else if(c=='\r')o+="\\r";else o+=c;}return o;}
static std::string field(const std::string&j,const std::string&k){std::string m="\""+k+"\":\"";auto p=j.find(m);if(p==std::string::npos)return{};p+=m.size();std::string o;bool s=false;for(;p<j.size();++p){char c=j[p];if(s){o+=c;s=false;continue;}if(c=='\\'){s=true;continue;}if(c=='"')break;o+=c;}return o;}
#ifdef _WIN32
static std::string wideBack(const wchar_t*w,int n){int sz=WideCharToMultiByte(CP_UTF8,0,w,n,nullptr,0,nullptr,nullptr);std::string s(sz,'\0');WideCharToMultiByte(CP_UTF8,0,w,n,s.data(),sz,nullptr,nullptr);return s;}
static AIResult http(const std::string&url,const std::string&body){AIResult a;std::string u=url;bool https=false;if(u.rfind("https://",0)==0){https=true;u.erase(0,8);}else if(u.rfind("http://",0)==0)u.erase(0,7);auto slash=u.find('/');std::wstring host;std::wstring path=L"/";std::string hs=slash==std::string::npos?u:u.substr(0,slash);if(slash!=std::string::npos){std::string ps=u.substr(slash);int n=MultiByteToWideChar(CP_UTF8,0,ps.data(),(int)ps.size(),nullptr,0);path.resize(n);MultiByteToWideChar(CP_UTF8,0,ps.data(),(int)ps.size(),path.data(),n);}int hn=MultiByteToWideChar(CP_UTF8,0,hs.data(),(int)hs.size(),nullptr,0);host.resize(hn);MultiByteToWideChar(CP_UTF8,0,hs.data(),(int)hs.size(),host.data(),hn);HINTERNET s=WinHttpOpen(L"CORE-AI/0.2",WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,WINHTTP_NO_PROXY_NAME,WINHTTP_NO_PROXY_BYPASS,0);if(!s){a.error="WinHTTP unavailable";return a;}HINTERNET c=WinHttpConnect(s,host.c_str(),https?INTERNET_DEFAULT_HTTPS_PORT:INTERNET_DEFAULT_HTTP_PORT,0);if(!c){WinHttpCloseHandle(s);a.error="connect failed";return a;}HINTERNET r=WinHttpOpenRequest(c,L"POST",path.c_str(),nullptr,WINHTTP_NO_REFERER,WINHTTP_DEFAULT_ACCEPT_TYPES,https?WINHTTP_FLAG_SECURE:0);if(!r){WinHttpCloseHandle(c);WinHttpCloseHandle(s);a.error="request failed";return a;}LPCWSTR h=L"Content-Type: application/json\r\nAccept: application/json\r\n";BOOL ok=WinHttpSendRequest(r,h,(DWORD)-1L,(LPVOID)body.data(),(DWORD)body.size(),(DWORD)body.size(),0)&&WinHttpReceiveResponse(r,nullptr);if(!ok){a.error="request failed";}else{a.ok=true;DWORD n=0;while(WinHttpQueryDataAvailable(r,&n)&&n){std::string b(n,'\0');DWORD g=0;if(!WinHttpReadData(r,b.data(),n,&g)||!g)break;b.resize(g);a.text+=b;}}WinHttpCloseHandle(r);WinHttpCloseHandle(c);WinHttpCloseHandle(s);if(!a.ok)return a;a.text=field(a.text,"content");a.ok=!a.text.empty();if(!a.ok)a.error="invalid Ollama response";return a;}
#else
static AIResult http(const std::string&url,const std::string&body){AIResult a;auto t=std::tmpnam(nullptr);std::string qp="'";for(char c:url){if(c=='\'')qp+="'\\''";else qp+=c;}qp+="'";std::string bp="'";for(char c:body){if(c=='\'')bp+="'\\''";else bp+=c;}bp+="'";std::string cmd="curl -sS --max-time 30 -X POST -H 'Content-Type: application/json' --data "+bp+" "+qp+" > '"+t+"'";int e=std::system(cmd.c_str());if(e!=0){a.error="curl failed";return a;}std::ifstream f(t);std::ostringstream x;x<<f.rdbuf();a.text=field(x.str(),"content");a.ok=!a.text.empty();a.error=a.ok?"":"invalid Ollama response";std::remove(t);return a;}
#endif
OllamaProvider::OllamaProvider():url_(E("CORE_OLLAMA_URL")),model_(E("CORE_MODEL")){if(url_.empty())url_="http://127.0.0.1:11434";if(model_.empty())model_="llama3.2";} OllamaProvider::OllamaProvider(std::string m):OllamaProvider(){if(!m.empty())model_=std::move(m);} AIResult OllamaProvider::complete(const AIRequest&q){std::string b="{\"model\":\""+esc(model_)+"\",\"messages\":[";if(!q.system.empty())b+="{\"role\":\"system\",\"content\":\""+esc(q.system)+"\"},";b+="{\"role\":\"user\",\"content\":\""+esc(q.prompt)+"\"}],\"stream\":false}";return http(url_+"/api/chat",b);} bool OllamaProvider::available()const{auto p=url_.rfind("http://",0)==0?url_.substr(7):url_;std::string cmd="curl -sS --max-time 2 '"+url_+"/api/tags' >nul 2>&1";return std::system(cmd.c_str())==0;}
}
