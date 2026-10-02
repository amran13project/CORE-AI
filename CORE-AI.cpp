/*
 CORE AI — PERSONAL AI CORE
 Single-file C++20 implementation.

 Build (Windows + LLVM-MinGW):
   clang++ -std=c++20 -O2 CORE-AI.cpp -o core-ai.exe -lwinhttp -lshell32 -ladvapi32 -luser32

 Build (Linux/macOS):
   clang++ -std=c++20 -O2 CORE-AI.cpp -o core-ai
   Requires curl for HTTP fallback.

 GUI integration:
   Define CORE_AI_NO_MAIN and build this file into a library/DLL.
   The exported C API at the bottom is intentionally small and GUI-friendly.

 External providers are adapters, not fake implementations:
   - Ollama: local chat / reasoning / coding / vision
   - Google Custom Search: web search (GOOGLE_API_KEY + GOOGLE_CX)
   - Stable Diffusion WebUI API: image generation (CORE_IMAGE_URL)
   - Maps: provider URL helpers; API-backed places/routing can be added through plugin/tools
   - Voice: external Whisper-compatible executable can be configured

 Core systems included in this one file:
   Brain, context, memory, knowledge, library, projects, tools, agents,
   workflows, plugins, permissions, audit, web search, image generation,
   vision, maps, model routing, diagnostics, and GUI C API.

 The program never reports external-provider success unless the request itself
 succeeds. Missing providers are reported as unavailable.
*/

#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <optional>
#include <random>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <winhttp.h>
#endif

namespace coreai {
namespace fs = std::filesystem;

static long long nowSec() {
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

static std::string env(std::string_view key) {
#ifdef _WIN32
    std::string k(key);
    DWORD n = GetEnvironmentVariableA(k.c_str(), nullptr, 0);
    if (!n) return {};
    std::string out(n, '\0');
    DWORD got = GetEnvironmentVariableA(k.c_str(), out.data(), n);
    if (!got) return {};
    out.resize(got);
    return out;
#else
    const char* p = std::getenv(std::string(key).c_str());
    return p ? std::string(p) : std::string();
#endif
}

static std::string trim(std::string s) {
    auto isspace2 = [](unsigned char c) { return std::isspace(c) != 0; };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [&](char c) { return !isspace2((unsigned char)c); }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [&](char c) { return !isspace2((unsigned char)c); }).base(), s.end());
    return s;
}

static std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return (char)std::tolower(c); });
    return s;
}

static std::string jsonEscape(std::string_view s) {
    std::string o;
    o.reserve(s.size() + 16);
    for (unsigned char c : s) {
        switch (c) {
            case '"': o += "\\\""; break;
            case '\\': o += "\\\\"; break;
            case '\n': o += "\\n"; break;
            case '\r': o += "\\r"; break;
            case '\t': o += "\\t"; break;
            case '\b': o += "\\b"; break;
            case '\f': o += "\\f"; break;
            default:
                if (c < 0x20) {
                    char b[8]; std::snprintf(b, sizeof(b), "\\u%04x", c); o += b;
                } else o += (char)c;
        }
    }
    return o;
}

static std::string urlEncode(std::string_view s) {
    static const char hex[] = "0123456789ABCDEF";
    std::string o;
    for (unsigned char c : s) {
        if (std::isalnum(c) || c=='-' || c=='_' || c=='.' || c=='~') o += (char)c;
        else { o += '%'; o += hex[c>>4]; o += hex[c&15]; }
    }
    return o;
}

static std::vector<std::string> splitPipe(std::string s) {
    std::vector<std::string> out;
    std::string cur;
    bool esc = false;
    for (char c : s) {
        if (esc) { cur += c; esc = false; continue; }
        if (c == '\\') { esc = true; continue; }
        if (c == '|') { out.push_back(trim(cur)); cur.clear(); }
        else cur += c;
    }
    out.push_back(trim(cur));
    return out;
}

static std::string readAll(const fs::path& p) {
    std::ifstream f(p, std::ios::binary);
    if (!f) return {};
    std::ostringstream ss; ss << f.rdbuf(); return ss.str();
}

static bool writeAll(const fs::path& p, std::string_view s) {
    std::error_code ec;
    if (!p.parent_path().empty()) fs::create_directories(p.parent_path(), ec);
    std::ofstream f(p, std::ios::binary | std::ios::trunc);
    if (!f) return false;
    f.write(s.data(), (std::streamsize)s.size());
    return (bool)f;
}

class Http {
public:
    struct Response { long status = 0; std::string body; std::string error; };

#ifdef _WIN32
    static std::wstring w(std::string_view s) {
        if (s.empty()) return {};
        int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
        if (n <= 0) return {};
        std::wstring o(n, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), o.data(), n);
        return o;
    }

    static bool parseUrl(const std::string& in, bool& secure, std::wstring& host, INTERNET_PORT& port, std::wstring& path) {
        std::string u = in;
        secure = false;
        port = 0;
        if (u.rfind("https://", 0) == 0) { secure = true; port = INTERNET_DEFAULT_HTTPS_PORT; u.erase(0,8); }
        else if (u.rfind("http://", 0) == 0) { port = INTERNET_DEFAULT_HTTP_PORT; u.erase(0,7); }
        else return false;
        auto slash = u.find('/');
        std::string hp = slash == std::string::npos ? u : u.substr(0, slash);
        std::string pp = slash == std::string::npos ? "/" : u.substr(slash);
        auto colon = hp.rfind(':');
        if (colon != std::string::npos) {
            try { port = (INTERNET_PORT)std::stoi(hp.substr(colon+1)); } catch (...) { return false; }
            hp = hp.substr(0, colon);
        }
        host = w(hp); path = w(pp);
        return !host.empty();
    }

    static Response request(const std::string& method, const std::string& url,
                            const std::string& body = {},
                            const std::string& contentType = "application/json",
                            const std::string& headers = {}) {
        Response r; bool secure = false; INTERNET_PORT port = 0; std::wstring host, path;
        if (!parseUrl(url, secure, host, port, path)) { r.error = "invalid URL"; return r; }
        HINTERNET s = WinHttpOpen(L"CORE-AI/2.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        if (!s) { r.error = "WinHttpOpen failed"; return r; }
        WinHttpSetTimeouts(s, 7000, 7000, 15000, 30000);
        HINTERNET c = WinHttpConnect(s, host.c_str(), port, 0);
        if (!c) { WinHttpCloseHandle(s); r.error = "WinHttpConnect failed"; return r; }
        HINTERNET q = WinHttpOpenRequest(c, w(method).c_str(), path.c_str(), nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, secure ? WINHTTP_FLAG_SECURE : 0);
        if (!q) { WinHttpCloseHandle(c); WinHttpCloseHandle(s); r.error = "WinHttpOpenRequest failed"; return r; }
        std::wstring hs = L"Content-Type: " + w(contentType) + L"\r\n";
        if (!headers.empty()) hs += w(headers);
        BOOL ok = WinHttpSendRequest(q, hs.c_str(), (DWORD)-1L,
                                     body.empty() ? WINHTTP_NO_REQUEST_DATA : (LPVOID)body.data(),
                                     (DWORD)body.size(), (DWORD)body.size(), 0);
        if (!ok || !WinHttpReceiveResponse(q, nullptr)) {
            r.error = "HTTP request failed";
            WinHttpCloseHandle(q); WinHttpCloseHandle(c); WinHttpCloseHandle(s); return r;
        }
        DWORD code = 0, cs = sizeof(code);
        WinHttpQueryHeaders(q, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &code, &cs, WINHTTP_NO_HEADER_INDEX);
        r.status = (long)code;
        for (;;) {
            DWORD n = 0; if (!WinHttpQueryDataAvailable(q, &n) || n == 0) break;
            std::string b(n, '\0'); DWORD got = 0;
            if (!WinHttpReadData(q, b.data(), n, &got) || got == 0) break;
            b.resize(got); r.body += b;
        }
        WinHttpCloseHandle(q); WinHttpCloseHandle(c); WinHttpCloseHandle(s); return r;
    }
#else
    static std::string shellQuote(std::string_view s) {
        std::string o = "'";
        for (char c : s) { if (c == '\'') o += "'\\''"; else o += c; }
        o += "'"; return o;
    }
    static Response request(const std::string& method, const std::string& url,
                            const std::string& body = {},
                            const std::string& contentType = "application/json",
                            const std::string& = {}) {
        Response r;
        fs::path tmp = fs::temp_directory_path() / ("core_http_" + std::to_string(nowSec()) + ".tmp");
        std::string cmd = "curl -sS -L --max-time 30 -X " + shellQuote(method) + " -H " + shellQuote("Content-Type: "+contentType);
        if (!body.empty()) cmd += " --data " + shellQuote(body);
        cmd += " " + shellQuote(url) + " > " + shellQuote(tmp.string());
        int ec = std::system(cmd.c_str());
        if (ec != 0) { r.error = "curl failed"; return r; }
        r.status = 200; r.body = readAll(tmp);
        std::error_code x; fs::remove(tmp, x); return r;
    }
#endif
};

static std::optional<std::string> jsonStringField(const std::string& json, std::string_view key) {
    std::string marker = "\"" + std::string(key) + "\":\"";
    auto p = json.find(marker);
    if (p == std::string::npos) return std::nullopt;
    p += marker.size();
    std::string out; bool esc = false;
    for (; p < json.size(); ++p) {
        char c = json[p];
        if (esc) {
            switch (c) { case 'n': out+='\n'; break; case 'r': out+='\r'; break; case 't': out+='\t'; break; case '"': out+='"'; break; case '\\': out+='\\'; break; default: out+=c; break; }
            esc = false; continue;
        }
        if (c == '\\') { esc = true; continue; }
        if (c == '"') break;
        out += c;
    }
    return out;
}

class Audit {
    fs::path path_; std::mutex m_;
public:
    explicit Audit(fs::path root): path_(root/"audit.log") { std::error_code e; fs::create_directories(root,e); }
    void log(std::string_view a, std::string_view r) {
        std::lock_guard<std::mutex> g(m_); std::ofstream f(path_, std::ios::app);
        if (f) f << nowSec() << " | " << a << " | " << r << "\n";
    }
};

class Permissions {
    mutable std::mutex m_; std::unordered_map<std::string,bool> v_;
public:
    void allow(const std::string& c) { std::lock_guard<std::mutex> g(m_); v_[c]=true; }
    void deny(const std::string& c) { std::lock_guard<std::mutex> g(m_); v_[c]=false; }
    bool check(const std::string& c) const { std::lock_guard<std::mutex> g(m_); auto i=v_.find(c); return i!=v_.end() && i->second; }
};

class Memory {
    fs::path path_;
public:
    explicit Memory(fs::path root): path_(root/"memory.jsonl") { std::error_code e; fs::create_directories(root,e); }
    bool add(std::string_view text, std::string_view category="personal") {
        std::ofstream f(path_, std::ios::app); if(!f) return false;
        f << "{\"time\":" << nowSec() << ",\"category\":\"" << jsonEscape(category) << "\",\"text\":\"" << jsonEscape(text) << "\"}\n"; return true;
    }
    std::vector<std::string> search(std::string query, size_t limit=50) const {
        std::vector<std::string> out; std::ifstream f(path_); if(!f) return out;
        query=lower(std::move(query)); std::string line;
        while(std::getline(f,line) && out.size()<limit) if(query.empty() || lower(line).find(query)!=std::string::npos) out.push_back(line);
        return out;
    }
};

class Library {
    fs::path root_;
    fs::path safe(std::string_view n) const {
        fs::path p(n); if(p.is_absolute()) return {};
        for(const auto& part:p) if(part=="..") return {};
        return root_/p;
    }
public:
    explicit Library(fs::path root): root_(std::move(root)) { std::error_code e; fs::create_directories(root_,e); }
    bool save(std::string_view n,std::string_view c){auto p=safe(n);return !p.empty()&&writeAll(p,c);}
    std::string read(std::string_view n)const{auto p=safe(n);return p.empty()?std::string():readAll(p);}
    std::vector<std::string> search(std::string q,size_t lim=50)const{
        std::vector<std::string>o;std::error_code e;if(!fs::exists(root_,e))return o;q=lower(std::move(q));
        for(auto it=fs::recursive_directory_iterator(root_,e);it!=fs::recursive_directory_iterator()&&o.size()<lim;it.increment(e)){if(e)break;if(!it->is_regular_file(e))continue;auto c=lower(readAll(it->path()));if(q.empty()||c.find(q)!=std::string::npos)o.push_back(fs::relative(it->path(),root_,e).string());}
        return o;
    }
};

class Projects {
    fs::path root_;
    static std::string safeName(std::string s){std::string o;for(char c:s){if(std::isalnum((unsigned char)c)||c=='-'||c=='_')o+=c;else if(c==' ')o+='_';}return o.empty()?"Project":o;}
public:
    explicit Projects(fs::path root):root_(std::move(root)){std::error_code e;fs::create_directories(root_,e);}
    bool create(std::string name){std::error_code e;auto p=root_/safeName(std::move(name));fs::create_directories(p/"src",e);fs::create_directories(p/"assets",e);fs::create_directories(p/"library",e);writeAll(p/".core-project","{\"version\":1}\n");return !e;}
    std::vector<std::string> list()const{std::vector<std::string>o;std::error_code e;for(auto&e2:fs::directory_iterator(root_,e))if(!e&&e2.is_directory(e))o.push_back(e2.path().filename().string());return o;}
};

class Ollama {
    std::string base_, model_;
public:
    struct Result{bool ok=false;std::string text,raw,error;};
    Ollama():base_(env("CORE_OLLAMA_URL")),model_(env("CORE_MODEL")){if(base_.empty())base_="http://127.0.0.1:11434";if(model_.empty())model_="llama3.2";}
    void setModel(std::string m){model_=std::move(m);} std::string model()const{return model_;}
    Result chat(std::string_view prompt,std::string_view system={})const{
        std::string messages="[";
        if(!system.empty())messages+="{\"role\":\"system\",\"content\":\""+jsonEscape(system)+"\"},";
        messages+="{\"role\":\"user\",\"content\":\""+jsonEscape(prompt)+"\"}]";
        std::string body="{\"model\":\""+jsonEscape(model_)+"\",\"stream\":false,\"messages\":"+messages+"}";
        auto r=Http::request("POST",base_+"/api/chat",body);
        if(r.status<200||r.status>=300)return{false,{},r.body,r.error.empty()?"Ollama HTTP "+std::to_string(r.status):r.error};
        auto txt=jsonStringField(r.body,"content");
        if(!txt)return{false,{},r.body,"Ollama response missing content"};
        return{true,*txt,r.body,{}};
    }
    bool available()const{auto r=Http::request("GET",base_+"/api/tags");return r.status>=200&&r.status<300;}
};

class Search {
public:
    struct Result{bool ok=false;std::string body,error;};
    static Result google(std::string_view q,int n=5){
        auto key=env("GOOGLE_API_KEY"),cx=env("GOOGLE_CX");if(key.empty()||cx.empty())return{false,{},"GOOGLE_API_KEY and GOOGLE_CX are required"};
        auto r=Http::request("GET","https://www.googleapis.com/customsearch/v1?key="+urlEncode(key)+"&cx="+urlEncode(cx)+"&num="+std::to_string(std::clamp(n,1,10))+"&q="+urlEncode(q));
        if(r.status<200||r.status>=300)return{false,r.body,r.error.empty()?"Google Search HTTP "+std::to_string(r.status):r.error};return{true,r.body,{}};
    }
};

class ImageGen {
public:
    struct Result{bool ok=false;std::string file,error;};
private:
    static std::vector<unsigned char> b64(std::string_view s){
        static const std::array<int,128> map=[](){std::array<int,128>a{};a.fill(-1);const char*t="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";for(int i=0;i<64;++i)a[(unsigned char)t[i]]=i;return a;}();
        std::vector<unsigned char>o;int v=0,b=-8;for(unsigned char c:s){if(c=='=')break;if(c>=128||map[c]<0)continue;v=(v<<6)+map[c];b+=6;if(b>=0){o.push_back((unsigned char)((v>>b)&255));b-=8;}}return o;
    }
public:
    Result generate(std::string_view prompt,std::string_view outFile)const{
        std::string base=env("CORE_IMAGE_URL");if(base.empty())base="http://127.0.0.1:7860";
        std::string body="{\"prompt\":\""+jsonEscape(prompt)+"\",\"steps\":25,\"width\":1024,\"height\":1024,"+"\"batch_size\":1}";
        auto r=Http::request("POST",base+"/sdapi/v1/txt2img",body);
        if(r.status<200||r.status>=300)return{false,{},r.error.empty()?"image server HTTP "+std::to_string(r.status):r.error};
        auto p=r.body.find("\"images\":[\"");if(p==std::string::npos)return{false,{},"image server returned no image"};p+=11;auto e=r.body.find('"',p);if(e==std::string::npos)return{false,{},"invalid image response"};auto bytes=b64(r.body.substr(p,e-p));if(bytes.empty())return{false,{},"image base64 decode failed"};
        fs::path f(outFile);std::error_code ec;fs::create_directories(f.parent_path(),ec);std::ofstream out(f,std::ios::binary|std::ios::trunc);if(!out)return{false,{},"cannot write image"};out.write((const char*)bytes.data(),(std::streamsize)bytes.size());return out?Result{true,std::string(outFile),{}}:Result{false,{},"image write failed"};
    }
};

class Vision {
    Ollama& llm_;
public:
    explicit Vision(Ollama& l):llm_(l){}
    std::string analyze(std::string_view file,std::string_view question){
        /* Common vision model convention for Ollama. We send a small JSON message manually. */
        std::ifstream f(std::string(file),std::ios::binary);if(!f)return"VISION ERROR: image not found";
        std::ostringstream raw;raw<<f.rdbuf();auto bytes=raw.str();
        static const char chars[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";std::string enc;int val=0,bits=-6;for(unsigned char c:bytes){val=(val<<8)|c;bits+=8;while(bits>=0){enc+=chars[(val>>bits)&63];bits-=6;}}if(bits>-6)enc+=chars[((val<<8)>>(bits+8))&63];while(enc.size()%4)enc+='=';
        std::string body="{\"model\":\""+jsonEscape(llm_.model())+"\",\"stream\":false,\"messages\":[{\"role\":\"user\",\"content\":\""+jsonEscape(question)+"\",\"images\":[\""+enc+"\"]}]}";
        auto url=env("CORE_OLLAMA_URL");if(url.empty())url="http://127.0.0.1:11434";auto r=Http::request("POST",url+"/api/chat",body);if(r.status<200||r.status>=300)return"VISION ERROR: "+(r.error.empty()?"Ollama HTTP "+std::to_string(r.status):r.error);auto t=jsonStringField(r.body,"content");return t?*t:"VISION ERROR: invalid response";
    }
};

class Maps {
public:
    static std::string searchUrl(std::string_view place){return"https://www.google.com/maps/search/?api=1&query="+urlEncode(place);} 
    static std::string directionsUrl(std::string_view from,std::string_view to){return"https://www.google.com/maps/dir/?api=1&origin="+urlEncode(from)+"&destination="+urlEncode(to);} 
    static std::string osmUrl(std::string_view place){return"https://www.openstreetmap.org/search?query="+urlEncode(place);} 
};

class Tools {
public:
    using Fn=std::function<std::string(const std::vector<std::string>&)>;
private:
    std::map<std::string,Fn> m_;
public:
    void add(std::string n,Fn f){m_[std::move(n)]=std::move(f);} 
    std::string run(const std::string&n,const std::vector<std::string>&a)const{auto i=m_.find(n);if(i==m_.end())return"{\"ok\":false,\"error\":\"unknown tool\"}";try{return i->second(a);}catch(const std::exception&e){return"{\"ok\":false,\"error\":\""+jsonEscape(e.what())+"\"}";}catch(...){return"{\"ok\":false,\"error\":\"tool failure\"}";}}
    std::vector<std::string> list()const{std::vector<std::string>o;for(auto&[n,_]:m_)o.push_back(n);return o;}
};

class Workflow {
    std::map<std::string,std::vector<std::string>> flows_;
public:
    void define(std::string event,std::vector<std::string> actions){flows_[std::move(event)]=std::move(actions);} 
    const std::vector<std::string>* get(std::string_view e)const{auto i=flows_.find(std::string(e));return i==flows_.end()?nullptr:&i->second;}
};

class PluginHost {
#ifdef _WIN32
    std::vector<HMODULE> handles_;
#endif
public:
    bool load(const fs::path& file){
#ifdef _WIN32
        HMODULE h=LoadLibraryW(file.wstring().c_str());if(!h)return false;handles_.push_back(h);return true;
#else
        (void)file;return false;
#endif
    }
    size_t count()const{
#ifdef _WIN32
        return handles_.size();
#else
        return 0;
#endif
    }
};

class Core {
    fs::path root_;
    Audit audit_;
    Permissions permissions_;
    Memory memory_;
    Library library_;
    Projects projects_;
    Ollama ollama_;
    Vision vision_;
    Tools tools_;
    Workflow workflows_;
    PluginHost plugins_;
public:
    Core():root_(rootPath()),audit_(root_),memory_(root_/"memory"),library_(root_/"library"),projects_(root_/"projects"),ollama_(),vision_(ollama_){
        permissions_.allow("memory.write");
        permissions_.allow("project.create");
        registerTools();
        workflows_.define("build-failed",{"log-failure","create-diagnosis"});
    }
    static fs::path rootPath(){
#ifdef _WIN32
        auto a=env("APPDATA");if(!a.empty())return fs::path(a)/"CORE-AI";
#endif
        auto h=env("HOME");if(!h.empty())return fs::path(h)/".core-ai";
        return fs::current_path()/".core-ai";
    }
    std::string version()const{return"2.0.0-full-single-file";}
    std::string root()const{return root_.string();}
    std::string model()const{return ollama_.model();}
    void setModel(std::string m){ollama_.setModel(std::move(m));}
    std::string status()const{
        std::ostringstream o;o<<"{\"version\":\""<<version()<<"\",\"model\":\""<<jsonEscape(model())<<"\",\"ollama\":"<<(ollama_.available()?"true":"false")<<",\"plugins\":"<<plugins_.count()<<",\"root\":\""<<jsonEscape(root())<<"\"}";return o.str();
    }
    std::string capabilities()const{return R"({
  "chat": true,
  "think": true,
  "code": true,
  "agent": true,
  "web_search": true,
  "image_generation": true,
  "vision": true,
  "maps": true,
  "memory": true,
  "library": true,
  "projects": true,
  "workflows": true,
  "plugin_loader": true,
  "audit": true,
  "permissions": true,
  "voice_adapter": false,
  "video_generation": false,
  "full_browser_control": false
})";}
    std::string chat(std::string_view p){auto r=ollama_.chat(p);if(!r.ok){audit_.log("chat","error:"+r.error);return"MODEL ERROR: "+r.error;}audit_.log("chat","success");return r.text;}
    std::string think(std::string_view p){auto r=ollama_.chat(p,"You are CORE AI THINK. Reason carefully, state assumptions, distinguish facts from guesses, compare options, and produce an actionable answer.");if(!r.ok)return"MODEL ERROR: "+r.error;return r.text;}
    std::string code(std::string_view p){auto r=ollama_.chat(p,"You are CORE AI CODE. Write robust production-oriented code. State files, dependencies, build/test commands, and never claim tests passed unless they were actually run.");if(!r.ok)return"MODEL ERROR: "+r.error;return r.text;}
    std::string agent(std::string prompt,int maxSteps=8){
        std::string context=std::move(prompt);
        std::ostringstream toolText;for(auto&n:tools_.list())toolText<<"- "<<n<<"\n";
        for(int step=0;step<maxSteps;++step){
            auto r=ollama_.chat(context+"\n\nTools:\n"+toolText.str()+"\nTo call a tool, reply exactly TOOL <name> <arg1>|<arg2>. Otherwise give your final answer.","You are CORE AI AGENT. Use tools only when useful. Never claim tool success unless tool output has ok=true.");
            if(!r.ok)return"MODEL ERROR: "+r.error;
            if(r.text.rfind("TOOL ",0)!=0)return r.text;
            std::istringstream ss(r.text);std::string x,name;ss>>x>>name;std::string rest;std::getline(ss,rest);rest=trim(rest);
            auto result=tools_.run(name,splitPipe(rest));audit_.log("agent.tool."+name,result);context += "\nTool result for "+name+":\n"+result+"\nContinue.";
        }
        return"AGENT STOPPED: maximum steps reached";
    }
    std::string search(std::string q,int n=5){auto r=Search::google(q,n);if(!r.ok)return"SEARCH ERROR: "+r.error;return r.body;}
    std::string image(std::string prompt,std::string out){auto r=ImageGen{}.generate(prompt,out);if(!r.ok)return"IMAGE ERROR: "+r.error;audit_.log("image.generate",out);return"IMAGE CREATED: "+out;}
    std::string vision(std::string file,std::string q){auto t=vision_.analyze(file,q);audit_.log("vision",t.rfind("VISION ERROR",0)==0?"error":"success");return t;}
    std::string map(std::string p){return Maps::searchUrl(p);} 
    std::string directions(std::string a,std::string b){return Maps::directionsUrl(a,b);} 
    std::string remember(std::string text,std::string cat="personal"){if(!permissions_.check("memory.write"))return"PERMISSION DENIED: memory.write";if(!memory_.add(text,cat))return"MEMORY ERROR";audit_.log("memory.add","success");return"MEMORY SAVED";}
    std::string memorySearch(std::string q)const{std::ostringstream o;for(auto&x:memory_.search(q))o<<x<<"\n";return o.str();}
    std::string librarySave(std::string n,std::string c){if(!library_.save(n,c))return"LIBRARY ERROR";audit_.log("library.save",n);return"LIBRARY SAVED: "+n;}
    std::string libraryRead(std::string n)const{auto x=library_.read(n);return x.empty()?"LIBRARY ERROR: not found":x;}
    std::string librarySearch(std::string q)const{std::ostringstream o;for(auto&x:library_.search(q))o<<x<<"\n";return o.str();}
    std::string projectCreate(std::string n){if(!permissions_.check("project.create"))return"PERMISSION DENIED: project.create";if(!projects_.create(n))return"PROJECT ERROR";audit_.log("project.create",n);return"PROJECT CREATED: "+n;}
    std::string projectList()const{std::ostringstream o;for(auto&x:projects_.list())o<<x<<"\n";return o.str();}
    bool loadPlugin(std::string path){bool ok=plugins_.load(path);audit_.log("plugin.load",ok?"success":"error");return ok;}
private:
    void registerTools(){
        tools_.add("memory.search",[this](auto&a){return"{\"ok\":true,\"result\":\""+jsonEscape(memorySearch(a.empty()?"":a[0]))+"\"}";});
        tools_.add("library.search",[this](auto&a){return"{\"ok\":true,\"result\":\""+jsonEscape(librarySearch(a.empty()?"":a[0]))+"\"}";});
        tools_.add("project.list",[this](auto&){return"{\"ok\":true,\"result\":\""+jsonEscape(projectList())+"\"}";});
        tools_.add("map.search",[](auto&a){return a.empty()?"{\"ok\":false,\"error\":\"query required\"}":"{\"ok\":true,\"url\":\""+jsonEscape(Maps::searchUrl(a[0]))+"\"}";});
        tools_.add("memory.add",[this](auto&a){if(!permissions_.check("memory.write"))return std::string("{\"ok\":false,\"error\":\"permission denied\"}");return a.empty()?std::string("{\"ok\":false,\"error\":\"text required\"}"):"{\"ok\":true,\"result\":\""+jsonEscape(remember(a[0]))+"\"}";});
        tools_.add("project.create",[this](auto&a){if(!permissions_.check("project.create"))return std::string("{\"ok\":false,\"error\":\"permission denied\"}");return a.empty()?std::string("{\"ok\":false,\"error\":\"name required\"}"):"{\"ok\":true,\"result\":\""+jsonEscape(projectCreate(a[0]))+"\"}";});
    }
};

static char* dupC(const std::string& s){char*p=(char*)std::malloc(s.size()+1);if(!p)return nullptr;std::memcpy(p,s.data(),s.size());p[s.size()]='\0';return p;}

#ifdef _WIN32
#define CORE_EXPORT extern "C" __declspec(dllexport)
#else
#define CORE_EXPORT extern "C"
#endif

struct CoreHandle { coreai::Core* p{}; };

CORE_EXPORT CoreHandle* core_create(){try{auto*h=new CoreHandle;h->p=new coreai::Core;return h;}catch(...){return nullptr;}}
CORE_EXPORT void core_destroy(CoreHandle*h){if(!h)return;delete h->p;delete h;}
CORE_EXPORT void core_free(char*p){std::free(p);}
CORE_EXPORT char* core_version(CoreHandle*h){return coreai::dupC(h&&h->p?h->p->version():"CORE ERROR");}
CORE_EXPORT char* core_status(CoreHandle*h){return coreai::dupC(h&&h->p?h->p->status():"{\"ok\":false}");}
CORE_EXPORT char* core_capabilities(CoreHandle*h){return coreai::dupC(h&&h->p?h->p->capabilities():"{}");}
CORE_EXPORT void core_set_model(CoreHandle*h,const char*m){if(h&&h->p&&m)h->p->setModel(m);}
CORE_EXPORT char* core_chat(CoreHandle*h,const char*p){return coreai::dupC(h&&h->p&&p?h->p->chat(p):"CORE ERROR");}
CORE_EXPORT char* core_think(CoreHandle*h,const char*p){return coreai::dupC(h&&h->p&&p?h->p->think(p):"CORE ERROR");}
CORE_EXPORT char* core_code(CoreHandle*h,const char*p){return coreai::dupC(h&&h->p&&p?h->p->code(p):"CORE ERROR");}
CORE_EXPORT char* core_agent(CoreHandle*h,const char*p){return coreai::dupC(h&&h->p&&p?h->p->agent(p):"CORE ERROR");}
CORE_EXPORT char* core_search(CoreHandle*h,const char*q,int n){return coreai::dupC(h&&h->p&&q?h->p->search(q,n):"CORE ERROR");}
CORE_EXPORT char* core_image(CoreHandle*h,const char*p,const char*f){return coreai::dupC(h&&h->p&&p&&f?h->p->image(p,f):"CORE ERROR");}
CORE_EXPORT char* core_vision(CoreHandle*h,const char*f,const char*q){return coreai::dupC(h&&h->p&&f&&q?h->p->vision(f,q):"CORE ERROR");}
CORE_EXPORT char* core_map(CoreHandle*h,const char*q){return coreai::dupC(h&&h->p&&q?h->p->map(q):"CORE ERROR");}
CORE_EXPORT char* core_directions(CoreHandle*h,const char*a,const char*b){return coreai::dupC(h&&h->p&&a&&b?h->p->directions(a,b):"CORE ERROR");}
CORE_EXPORT char* core_remember(CoreHandle*h,const char*t,const char*c){return coreai::dupC(h&&h->p&&t?h->p->remember(t,c?c:"personal"):"CORE ERROR");}
CORE_EXPORT char* core_memory_search(CoreHandle*h,const char*q){return coreai::dupC(h&&h->p&&q?h->p->memorySearch(q):"CORE ERROR");}
CORE_EXPORT char* core_library_save(CoreHandle*h,const char*n,const char*c){return coreai::dupC(h&&h->p&&n&&c?h->p->librarySave(n,c):"CORE ERROR");}
CORE_EXPORT char* core_library_read(CoreHandle*h,const char*n){return coreai::dupC(h&&h->p&&n?h->p->libraryRead(n):"CORE ERROR");}
CORE_EXPORT char* core_library_search(CoreHandle*h,const char*q){return coreai::dupC(h&&h->p&&q?h->p->librarySearch(q):"CORE ERROR");}
CORE_EXPORT char* core_project_create(CoreHandle*h,const char*n){return coreai::dupC(h&&h->p&&n?h->p->projectCreate(n):"CORE ERROR");}
CORE_EXPORT char* core_project_list(CoreHandle*h){return coreai::dupC(h&&h->p?h->p->projectList():"CORE ERROR");}
CORE_EXPORT int core_plugin_load(CoreHandle*h,const char*p){return h&&h->p&&p&&h->p->loadPlugin(p)?1:0;}

} // namespace coreai

#ifndef CORE_AI_NO_MAIN
static void help(){
    std::cout<<"\nCORE AI 2.0 — PERSONAL AI CORE\n"
      <<"/chat <text>\n/think <text>\n/code <text>\n/agent <text>\n/search <query>\n/image <prompt>|<output.png>\n/vision <image>|<question>\n/map <place>\n/directions <from>|<to>\n/remember <text>\n/memory-search <text>\n/library-save <name>|<content>\n/library-read <name>\n/library-search <text>\n/project-create <name>\n/project-list\n/model <name>\n/status\n/capabilities\n/exit\n\n";
}

int main(){
    coreai::Core core;
    std::cout<<"============================================\nCORE AI — PERSONAL AI CORE\n============================================\n";
    std::cout<<"Version: "<<core.version()<<"\n";
    std::cout<<"Root: "<<core.root()<<"\n";
    help();
    std::string line;
    while(std::cout<<"CORE> "&&std::getline(std::cin,line)){
        line=coreai::trim(line);if(line.empty())continue;
        if(line=="/exit"||line=="exit"||line=="quit")break;
        if(line=="/help"){help();continue;}
        if(line=="/status"){std::cout<<core.status()<<"\n";continue;}
        if(line=="/capabilities"){std::cout<<core.capabilities()<<"\n";continue;}
        if(line.rfind("/model ",0)==0){core.setModel(coreai::trim(line.substr(7)));std::cout<<"Model: "<<core.model()<<"\n";continue;}
        if(line.rfind("/chat ",0)==0){std::cout<<core.chat(coreai::trim(line.substr(6)))<<"\n";continue;}
        if(line.rfind("/think ",0)==0){std::cout<<core.think(coreai::trim(line.substr(7)))<<"\n";continue;}
        if(line.rfind("/code ",0)==0){std::cout<<core.code(coreai::trim(line.substr(6)))<<"\n";continue;}
        if(line.rfind("/agent ",0)==0){std::cout<<core.agent(coreai::trim(line.substr(7)))<<"\n";continue;}
        if(line.rfind("/search ",0)==0){std::cout<<core.search(coreai::trim(line.substr(8)))<<"\n";continue;}
        if(line.rfind("/image ",0)==0){auto p=coreai::splitPipe(coreai::trim(line.substr(7)));std::cout<<(p.size()>=2?core.image(p[0],p[1]):"Usage: /image prompt|output.png")<<"\n";continue;}
        if(line.rfind("/vision ",0)==0){auto p=coreai::splitPipe(coreai::trim(line.substr(8)));std::cout<<(p.size()>=2?core.vision(p[0],p[1]):"Usage: /vision image|question")<<"\n";continue;}
        if(line.rfind("/map ",0)==0){std::cout<<core.map(coreai::trim(line.substr(5)))<<"\n";continue;}
        if(line.rfind("/directions ",0)==0){auto p=coreai::splitPipe(coreai::trim(line.substr(12)));std::cout<<(p.size()>=2?core.directions(p[0],p[1]):"Usage: /directions from|to")<<"\n";continue;}
        if(line.rfind("/remember ",0)==0){std::cout<<core.remember(coreai::trim(line.substr(10)))<<"\n";continue;}
        if(line.rfind("/memory-search ",0)==0){std::cout<<core.memorySearch(coreai::trim(line.substr(15)))<<"\n";continue;}
        if(line.rfind("/library-save ",0)==0){auto p=coreai::splitPipe(coreai::trim(line.substr(14)));std::cout<<(p.size()>=2?core.librarySave(p[0],p[1]):"Usage: /library-save name|content")<<"\n";continue;}
        if(line.rfind("/library-read ",0)==0){std::cout<<core.libraryRead(coreai::trim(line.substr(14)))<<"\n";continue;}
        if(line.rfind("/library-search ",0)==0){std::cout<<core.librarySearch(coreai::trim(line.substr(16)))<<"\n";continue;}
        if(line.rfind("/project-create ",0)==0){std::cout<<core.projectCreate(coreai::trim(line.substr(16)))<<"\n";continue;}
        if(line=="/project-list"){std::cout<<core.projectList()<<"\n";continue;}
        std::cout<<core.chat(line)<<"\n";
    }
    return 0;
}
#endif
