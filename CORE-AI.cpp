// CORE AI — PERSONAL AI CORE
// Single-file C++20 core. GUI can be built separately against the exported C API.
// Real integrations only: unavailable external providers are reported as unavailable.
// Windows: link WinHTTP. POSIX: requires curl for HTTP fallback.
//
// Windows example (LLVM-MinGW):
// clang++ -std=c++20 -O2 CORE-AI.cpp -o core-ai.exe -mconsole -static-libgcc -static-libstdc++ -lwinhttp -lshell32 -ladvapi32 -luser32
// GUI library example:
// clang++ -std=c++20 -O2 -DCORE_AI_NO_MAIN -shared CORE-AI.cpp -o core-ai.dll -lwinhttp

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <mutex>
#include <memory>
#include <thread>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
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
#else
#include <dlfcn.h>
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
    std::string v(n, '\0');
    DWORD got = GetEnvironmentVariableA(k.c_str(), v.data(), n);
    if (!got) return {};
    v.resize(got);
    return v;
#else
    const char* p = std::getenv(std::string(key).c_str());
    return p ? std::string(p) : std::string();
#endif
}

static std::string trim(std::string s) {
    auto ws = [](unsigned char c) { return std::isspace(c) != 0; };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [&](char c){ return !ws((unsigned char)c); }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [&](char c){ return !ws((unsigned char)c); }).base(), s.end());
    return s;
}

static std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return (char)std::tolower(c); });
    return s;
}

static std::string jsonEscape(std::string_view s) {
    std::string o;
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
                if (c < 0x20) { char b[8]; std::snprintf(b, sizeof(b), "\\u%04x", c); o += b; }
                else o += (char)c;
        }
    }
    return o;
}

static std::string urlEncode(std::string_view s) {
    static const char hex[] = "0123456789ABCDEF";
    std::string o;
    for (unsigned char c : s) {
        if (std::isalnum(c) || c=='-' || c=='_' || c=='.' || c=='~') o += (char)c;
        else { o += '%'; o += hex[c >> 4]; o += hex[c & 15]; }
    }
    return o;
}

static std::vector<std::string> splitPipe(std::string_view s) {
    std::vector<std::string> out;
    std::string cur;
    bool escaped = false;
    for (char c : s) {
        if (escaped) { cur += c; escaped = false; continue; }
        if (c == '\\') { escaped = true; continue; }
        if (c == '|') { out.push_back(trim(cur)); cur.clear(); }
        else cur += c;
    }
    out.push_back(trim(cur));
    return out;
}

static std::string readAll(const fs::path& p) {
    std::ifstream f(p, std::ios::binary);
    if (!f) return {};
    std::ostringstream s; s << f.rdbuf(); return s.str();
}

static bool writeAll(const fs::path& p, std::string_view data) {
    std::error_code ec;
    if (!p.parent_path().empty()) fs::create_directories(p.parent_path(), ec);
    std::ofstream f(p, std::ios::binary | std::ios::trunc);
    if (!f) return false;
    f.write(data.data(), (std::streamsize)data.size());
    return (bool)f;
}

class Http {
public:
    struct Response { long status = 0; std::string body; std::string error; };
#ifdef _WIN32
    static std::wstring wide(std::string_view s) {
        if (s.empty()) return {};
        int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
        if (n <= 0) return {};
        std::wstring w(n, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), w.data(), n);
        return w;
    }
    static bool parseUrl(const std::string& u, bool& https, std::wstring& host, INTERNET_PORT& port, std::wstring& path) {
        std::string s = u;
        https = false;
        if (s.rfind("https://", 0) == 0) { https = true; s.erase(0, 8); port = INTERNET_DEFAULT_HTTPS_PORT; }
        else if (s.rfind("http://", 0) == 0) { port = INTERNET_DEFAULT_HTTP_PORT; s.erase(0, 7); }
        else return false;
        auto slash = s.find('/');
        std::string hp = slash == std::string::npos ? s : s.substr(0, slash);
        std::string pp = slash == std::string::npos ? "/" : s.substr(slash);
        auto colon = hp.rfind(':');
        if (colon != std::string::npos) {
            try { port = (INTERNET_PORT)std::stoi(hp.substr(colon+1)); }
            catch (...) { return false; }
            hp.erase(colon);
        }
        host = wide(hp); path = wide(pp);
        return !host.empty();
    }
    static Response request(const std::string& method, const std::string& url,
                            const std::string& body = {},
                            const std::string& contentType = "application/json",
                            const std::string& extraHeaders = {}) {
        Response r;
        bool https = false; INTERNET_PORT port = 0; std::wstring host, path;
        if (!parseUrl(url, https, host, port, path)) { r.error = "invalid URL"; return r; }
        HINTERNET session = WinHttpOpen(L"CORE-AI/2.1", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        if (!session) { r.error = "WinHttpOpen failed"; return r; }
        WinHttpSetTimeouts(session, 7000, 7000, 15000, 30000);
        HINTERNET conn = WinHttpConnect(session, host.c_str(), port, 0);
        if (!conn) { WinHttpCloseHandle(session); r.error = "WinHttpConnect failed"; return r; }
        HINTERNET req = WinHttpOpenRequest(conn, wide(method).c_str(), path.c_str(), nullptr,
                                           WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
                                           https ? WINHTTP_FLAG_SECURE : 0);
        if (!req) { WinHttpCloseHandle(conn); WinHttpCloseHandle(session); r.error = "WinHttpOpenRequest failed"; return r; }
        std::wstring headers = L"Content-Type: " + wide(contentType) + L"\r\nAccept: application/json\r\n";
        if (!extraHeaders.empty()) headers += wide(extraHeaders);
        BOOL ok = WinHttpSendRequest(req, headers.c_str(), (DWORD)-1L,
                                     body.empty() ? WINHTTP_NO_REQUEST_DATA : (LPVOID)body.data(),
                                     (DWORD)body.size(), (DWORD)body.size(), 0);
        if (ok) ok = WinHttpReceiveResponse(req, nullptr);
        if (!ok) {
            r.error = "HTTP request failed";
            WinHttpCloseHandle(req); WinHttpCloseHandle(conn); WinHttpCloseHandle(session); return r;
        }
        DWORD status = 0, size = sizeof(status);
        WinHttpQueryHeaders(req, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                            WINHTTP_HEADER_NAME_BY_INDEX, &status, &size, WINHTTP_NO_HEADER_INDEX);
        r.status = (long)status;
        for (;;) {
            DWORD avail = 0;
            if (!WinHttpQueryDataAvailable(req, &avail) || avail == 0) break;
            std::string chunk(avail, '\0'); DWORD got = 0;
            if (!WinHttpReadData(req, chunk.data(), avail, &got) || got == 0) break;
            chunk.resize(got); r.body += chunk;
        }
        WinHttpCloseHandle(req); WinHttpCloseHandle(conn); WinHttpCloseHandle(session);
        return r;
    }
#else
    static std::string quote(std::string_view s) {
        std::string q = "'";
        for (char c : s) { if (c == '\'') q += "'\\''"; else q += c; }
        q += "'";
        return q;
    }
    static Response request(const std::string& method, const std::string& url,
                            const std::string& body = {},
                            const std::string& contentType = "application/json",
                            const std::string& = {}) {
        Response r;
        fs::path tmp = fs::temp_directory_path() / ("core_ai_http_" + std::to_string(nowSec()) + ".tmp");
        std::string cmd = "curl -sS -L --max-time 30 -X " + quote(method) +
                          " -H " + quote("Content-Type: " + contentType);
        if (!body.empty()) cmd += " --data " + quote(body);
        cmd += " " + quote(url) + " > " + quote(tmp.string());
        int ec = std::system(cmd.c_str());
        if (ec != 0) { r.error = "curl failed"; return r; }
        r.status = 200; r.body = readAll(tmp);
        std::error_code x; fs::remove(tmp, x);
        return r;
    }
#endif
};

static std::optional<std::string> jsonStringField(const std::string& json, std::string_view key) {
    const std::string marker = "\"" + std::string(key) + "\":\"";
    auto p = json.find(marker);
    if (p == std::string::npos) return std::nullopt;
    p += marker.size();
    std::string out;
    bool slash = false;
    for (; p < json.size(); ++p) {
        char c = json[p];
        if (slash) {
            switch (c) { case 'n': out += '\n'; break; case 'r': out += '\r'; break; case 't': out += '\t'; break; default: out += c; break; }
            slash = false; continue;
        }
        if (c == '\\') { slash = true; continue; }
        if (c == '"') break;
        out += c;
    }
    return out;
}

static std::string rootPath() {
#ifdef _WIN32
    std::string a = env("APPDATA");
    if (!a.empty()) return a + "\\CORE-AI";
    return ".\\CORE-AI";
#else
    std::string h = env("HOME");
    return (h.empty() ? "." : h) + std::string("/.core-ai");
#endif
}

class Audit {
    fs::path file_;
    mutable std::mutex m_;
public:
    explicit Audit(const fs::path& root) : file_(root / "audit.log") {}
    void log(std::string action, std::string result) {
        std::lock_guard<std::mutex> g(m_);
        std::ofstream f(file_, std::ios::app);
        if (f) f << nowSec() << "\t" << action << "\t" << result << "\n";
    }
};

class Permissions {
    std::unordered_map<std::string, bool> allow_;
public:
    Permissions() {
        allow_["memory.write"] = true;
        allow_["library.write"] = true;
        allow_["project.create"] = true;
        allow_["process.run"] = false;
        allow_["plugin.load"] = false;
    }
    bool check(const std::string& key) const {
        auto it = allow_.find(key);
        return it != allow_.end() && it->second;
    }
    void set(const std::string& key, bool value) { allow_[key] = value; }
    std::string json() const {
        std::ostringstream o; o << "{";
        bool first = true;
        for (const auto& [k,v] : allow_) { if (!first) o << ","; first = false; o << "\"" << jsonEscape(k) << "\":" << (v?"true":"false"); }
        o << "}"; return o.str();
    }
};

class Memory {
    fs::path file_;
    mutable std::mutex m_;
public:
    explicit Memory(const fs::path& root) : file_(root / "memory.jsonl") { fs::create_directories(file_.parent_path()); }
    bool add(std::string text, std::string category) {
        std::lock_guard<std::mutex> g(m_);
        std::ofstream f(file_, std::ios::app);
        if (!f) return false;
        f << "{\"time\":" << nowSec() << ",\"category\":\"" << jsonEscape(category)
          << "\",\"text\":\"" << jsonEscape(text) << "\"}\n";
        return true;
    }
    std::vector<std::string> search(std::string q) const {
        std::vector<std::string> out; std::ifstream f(file_); if (!f) return out;
        q = lower(q); std::string line;
        while (std::getline(f, line)) if (q.empty() || lower(line).find(q) != std::string::npos) out.push_back(line);
        return out;
    }
};

class Library {
    fs::path dir_;
public:
    explicit Library(const fs::path& root) : dir_(root / "library") { fs::create_directories(dir_); }
    bool save(const std::string& name, const std::string& content) {
        std::string safe = name;
        for (char& c : safe) if (!(std::isalnum((unsigned char)c) || c=='-' || c=='_' || c=='.')) c = '_';
        if (safe.empty()) return false;
        return writeAll(dir_ / safe, content);
    }
    std::string read(const std::string& name) const { return readAll(dir_ / name); }
    std::vector<std::string> search(const std::string& q) const {
        std::vector<std::string> out; std::string lq = lower(q);
        std::error_code ec;
        for (auto it = fs::directory_iterator(dir_, ec); !ec && it != fs::directory_iterator(); it.increment(ec)) {
            if (!it->is_regular_file(ec)) continue;
            std::string n = it->path().filename().string();
            if (lq.empty() || lower(n).find(lq) != std::string::npos || lower(readAll(it->path())).find(lq) != std::string::npos) out.push_back(n);
        }
        return out;
    }
};

class Projects {
    fs::path dir_;
public:
    explicit Projects(const fs::path& root) : dir_(root / "projects") { fs::create_directories(dir_); }
    bool create(std::string name) {
        for (char& c : name) if (!(std::isalnum((unsigned char)c) || c=='-' || c=='_')) c = '_';
        if (name.empty()) return false;
        fs::path p = dir_ / name;
        std::error_code ec; fs::create_directories(p, ec);
        return !ec && writeAll(p / "project.json", "{\"name\":\"" + jsonEscape(name) + "\",\"version\":1}\n");
    }
    std::vector<std::string> list() const {
        std::vector<std::string> out; std::error_code ec;
        for (auto it = fs::directory_iterator(dir_, ec); !ec && it != fs::directory_iterator(); it.increment(ec)) if (it->is_directory(ec)) out.push_back(it->path().filename().string());
        std::sort(out.begin(), out.end()); return out;
    }
};

class Ollama {
    std::string url_;
    std::string model_;
public:
    Ollama() : url_(env("CORE_OLLAMA_URL")), model_(env("CORE_MODEL")) {
        if (url_.empty()) url_ = "http://127.0.0.1:11434";
        if (model_.empty()) model_ = "llama3.2";
    }
    void setModel(const std::string& m) { if (!m.empty()) model_ = m; }
    const std::string& model() const { return model_; }
    Http::Response chat(const std::string& prompt, const std::string& system = {}, const std::string& imageB64 = {}) const {
        std::string msg = "{\"role\":\"user\",\"content\":\"" + jsonEscape(prompt) + "\"";
        if (!imageB64.empty()) msg += ",\"images\":[\"" + imageB64 + "\"]";
        msg += "}";
        std::string body = "{\"model\":\"" + jsonEscape(model_) + "\",\"messages\":[";
        if (!system.empty()) body += "{\"role\":\"system\",\"content\":\"" + jsonEscape(system) + "\"},";
        body += msg + "],\"stream\":false}";
        return Http::request("POST", url_ + "/api/chat", body);
    }
    bool available() const { return Http::request("GET", url_ + "/api/tags").status >= 200 && Http::request("GET", url_ + "/api/tags").status < 500; }
};

class Search {
public:
    static Http::Response google(const std::string& q, int n) {
        std::string key = env("GOOGLE_API_KEY"), cx = env("GOOGLE_CX");
        if (key.empty() || cx.empty()) return {0, {}, "GOOGLE_API_KEY and GOOGLE_CX are required"};
        std::string u = "https://www.googleapis.com/customsearch/v1?key=" + urlEncode(key) + "&cx=" + urlEncode(cx) + "&q=" + urlEncode(q) + "&num=" + std::to_string(std::clamp(n,1,10));
        return Http::request("GET", u);
    }
    static std::string webUrl(const std::string& q) { return "https://www.google.com/search?q=" + urlEncode(q); }
};

static std::vector<unsigned char> base64Decode(std::string_view s) {
    static const std::string table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::vector<unsigned char> out; int val=0, bits=-8;
    for (unsigned char c : s) {
        if (std::isspace(c) || c=='=') continue;
        size_t p = table.find((char)c); if (p == std::string::npos) continue;
        val = (val << 6) + (int)p; bits += 6;
        if (bits >= 0) { out.push_back((unsigned char)((val >> bits) & 0xFF)); bits -= 8; }
    }
    return out;
}

class ImageGen {
public:
    static std::string endpoint() { return env("CORE_IMAGE_URL").empty() ? "http://127.0.0.1:7860" : env("CORE_IMAGE_URL"); }
    static std::string generate(const std::string& prompt, const std::string& output) {
        std::string body = "{\"prompt\":\"" + jsonEscape(prompt) + "\",\"steps\":20}";
        auto r = Http::request("POST", endpoint() + "/sdapi/v1/txt2img", body);
        if (r.status < 200 || r.status >= 300) return "IMAGE ERROR: " + (r.error.empty() ? r.body : r.error);
        auto marker = r.body.find("\"images\":[\"");
        if (marker == std::string::npos) return "IMAGE ERROR: provider returned no image";
        marker += 11;
        auto end = r.body.find('"', marker);
        if (end == std::string::npos) return "IMAGE ERROR: invalid image response";
        auto bytes = base64Decode(r.body.substr(marker, end-marker));
        if (bytes.empty() || !writeAll(output, std::string_view((const char*)bytes.data(), bytes.size()))) return "IMAGE ERROR: could not write output";
        return "IMAGE CREATED: " + output;
    }
};

class Maps {
public:
    static std::string search(const std::string& place) { return "https://www.google.com/maps/search/?api=1&query=" + urlEncode(place); }
    static std::string directions(const std::string& from, const std::string& to) {
        return "https://www.google.com/maps/dir/?api=1&origin=" + urlEncode(from) + "&destination=" + urlEncode(to);
    }
    static std::string openStreetMap(const std::string& place) { return "https://www.openstreetmap.org/search?query=" + urlEncode(place); }
};

class Tools {
    using Fn = std::function<std::string(const std::vector<std::string>&)>;
    std::unordered_map<std::string, Fn> map_;
public:
    void add(std::string name, Fn fn) { map_[std::move(name)] = std::move(fn); }
    std::string run(const std::string& name, const std::vector<std::string>& args) const {
        auto it = map_.find(name); if (it == map_.end()) return "{\"ok\":false,\"error\":\"unknown tool\"}";
        try { return it->second(args); } catch (const std::exception& e) { return std::string("{\"ok\":false,\"error\":\"") + jsonEscape(e.what()) + "\"}"; }
    }
    std::vector<std::string> names() const { std::vector<std::string> o; for (auto& kv : map_) o.push_back(kv.first); std::sort(o.begin(), o.end()); return o; }
};

class Workflow {
    std::unordered_map<std::string, std::vector<std::string>> rules_;
public:
    void set(std::string event, std::vector<std::string> actions) { rules_[std::move(event)] = std::move(actions); }
    std::vector<std::string> get(const std::string& event) const { auto it = rules_.find(event); return it==rules_.end() ? std::vector<std::string>{} : it->second; }
};

class PluginHost {
    std::vector<std::string> loaded_;
public:
    bool load(const std::string& path) {
#ifdef _WIN32
        HMODULE h = LoadLibraryA(path.c_str());
        if (!h) return false;
#else
        void* h = dlopen(path.c_str(), RTLD_NOW);
        if (!h) return false;
#endif
        loaded_.push_back(path);
        return true;
    }
    size_t count() const { return loaded_.size(); }
};

class Core {
    fs::path root_;
    Audit audit_;
    Permissions permissions_;
    Memory memory_;
    Library library_;
    Projects projects_;
    Ollama ollama_;
    Tools tools_;
    Workflow workflow_;
    PluginHost plugins_;
    mutable std::mutex m_;

    static std::string csv(const std::vector<std::string>& v) {
        std::ostringstream o; for (size_t i=0;i<v.size();++i) { if(i)o<<","; o<<v[i]; } return o.str();
    }

    void registerTools() {
        tools_.add("memory.search", [this](const auto& a){ return jsonResult(memorySearch(a.empty()?"":a[0])); });
        tools_.add("memory.add", [this](const auto& a){ if(a.empty()) return std::string("{\"ok\":false,\"error\":\"text required\"}"); return jsonResult(remember(a[0], "agent")); });
        tools_.add("library.search", [this](const auto& a){ return jsonResult(librarySearch(a.empty()?"":a[0])); });
        tools_.add("project.list", [this](const auto&){ return jsonResult(projectList()); });
        tools_.add("project.create", [this](const auto& a){ return jsonResult(a.empty()?"PROJECT ERROR: name required":projectCreate(a[0])); });
        tools_.add("map.search", [](const auto& a){ if(a.empty()) return std::string("{\"ok\":false,\"error\":\"query required\"}"); return std::string("{\"ok\":true,\"url\":\"")+jsonEscape(Maps::search(a[0]))+"\"}"; });
        tools_.add("maps.directions", [](const auto& a){ if(a.size()<2) return std::string("{\"ok\":false,\"error\":\"from and to required\"}"); return std::string("{\"ok\":true,\"url\":\"")+jsonEscape(Maps::directions(a[0],a[1]))+"\"}"; });
    }

    static std::string jsonResult(std::string_view s) { return "{\"ok\":true,\"result\":\"" + jsonEscape(s) + "\"}"; }

public:
    Core() : root_(rootPath()), audit_(root_), memory_(root_), library_(root_), projects_(root_) {
        std::error_code ec; fs::create_directories(root_, ec);
        registerTools();
        audit_.log("core.start", version());
    }
    ~Core() { audit_.log("core.stop", "success"); }

    std::string version() const { return "2.2.0-full-single-file"; }
    std::string root() const { return root_.string(); }
    std::string model() const { return ollama_.model(); }
    void setModel(const std::string& m) { ollama_.setModel(m); audit_.log("model.set", m); }

    std::string capabilities() const {
        return "{\"chat\":true,\"think\":true,\"code\":true,\"agent\":true,\"web_search\":true,\"image_generation\":true,\"vision\":true,\"maps\":true,\"memory\":true,\"library\":true,\"projects\":true,\"workflows\":true,\"plugins\":true,\"process_tool\":true,\"permissions\":true,\"audit\":true,\"gui_c_api\":true,\"voice\":false,\"video_generation\":false,\"full_browser_control\":false}";
    }

    std::string status() const {
        bool ollama = false;
        auto r = Http::request("GET", env("CORE_OLLAMA_URL").empty()?"http://127.0.0.1:11434/api/tags":env("CORE_OLLAMA_URL")+"/api/tags");
        ollama = r.status >= 200 && r.status < 300;
        std::ostringstream o;
        o << "{\"version\":\"" << version() << "\",\"model\":\"" << jsonEscape(model())
          << "\",\"ollama\":" << (ollama?"true":"false")
          << ",\"plugins\":" << plugins_.count() << ",\"permissions\":" << permissions_.json()
          << ",\"root\":\"" << jsonEscape(root()) << "\"}";
        return o.str();
    }

    std::string chat(const std::string& prompt) {
        std::lock_guard<std::mutex> g(m_);
        auto r = ollama_.chat(prompt,
            "You are CORE AI, a personal AI core. Be accurate. Do not claim tools, providers, files, or actions succeeded unless they actually did. When a capability is unavailable, say so clearly.");
        if (r.status < 200 || r.status >= 300) return "MODEL ERROR: " + (r.error.empty()?r.body:r.error);
        auto text = jsonStringField(r.body, "content");
        if (!text) return "MODEL ERROR: invalid Ollama response";
        audit_.log("chat", "success");
        return *text;
    }

    std::string think(const std::string& task) {
        return chat("Reason through this task carefully and give the useful conclusion. Do not expose private chain-of-thought. Task: " + task);
    }

    std::string code(const std::string& task) {
        return chat("Act as a coding assistant. Produce practical, correct code and explain key decisions without exposing private chain-of-thought. Task: " + task);
    }

    std::string agent(const std::string& task) {
        std::string context = "Task: " + task + "\nAvailable tools: " + csv(tools_.names()) +
                              "\nWhen you need a tool, output exactly one line: TOOL <name> <arg1>|<arg2>. Otherwise answer normally.";
        for (int step=0; step<6; ++step) {
            auto r = ollama_.chat(context, "You are CORE AI's tool-using agent. Use only listed tools. Never claim a tool ran when it did not.");
            if (r.status < 200 || r.status >= 300) return "AGENT ERROR: " + (r.error.empty()?r.body:r.error);
            auto text = jsonStringField(r.body, "content");
            if (!text) return "AGENT ERROR: invalid model response";
            auto pos = text->find("TOOL ");
            if (pos == std::string::npos) { audit_.log("agent.done", "success"); return *text; }
            std::string line = text->substr(pos + 5);
            auto nl = line.find_first_of("\r\n"); if (nl != std::string::npos) line.erase(nl);
            auto sp = line.find(' '); if (sp == std::string::npos) return *text;
            std::string name = trim(line.substr(0, sp));
            auto args = splitPipe(trim(line.substr(sp+1)));
            auto result = tools_.run(name, args);
            audit_.log("agent.tool." + name, result);
            context += "\nTool result " + name + ": " + result + "\nContinue the task.";
        }
        return "AGENT ERROR: maximum agent steps reached";
    }

    std::string search(const std::string& q, int n=5) {
        auto r = Search::google(q,n);
        if (r.status < 200 || r.status >= 300) return "SEARCH ERROR: " + (r.error.empty()?r.body:r.error);
        audit_.log("web.search", "success"); return r.body;
    }

    std::string image(const std::string& prompt, const std::string& output) {
        if (prompt.empty() || output.empty()) return "IMAGE ERROR: prompt and output required";
        auto r = ImageGen::generate(prompt, output);
        audit_.log("image.generate", r.rfind("IMAGE CREATED:",0)==0?"success":"error");
        return r;
    }

    std::string vision(const std::string& file, const std::string& question) {
        std::string data = readAll(file);
        if (data.empty()) return "VISION ERROR: image file not found or empty";
        std::string b64;
        static const char tbl[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        int val=0,bits=-6;
        for (unsigned char c: data) { val=(val<<8)|c; bits+=8; while(bits>=0){ b64+=tbl[(val>>bits)&63]; bits-=6; } }
        if(bits>-6) b64+=tbl[((val<<8)>>(bits+8))&63]; while(b64.size()%4)b64+='=';
        auto r = ollama_.chat(question, "Analyze the supplied image and answer the question. Do not invent visual details.", b64);
        if (r.status < 200 || r.status >= 300) return "VISION ERROR: " + (r.error.empty()?r.body:r.error);
        auto text = jsonStringField(r.body,"content"); return text ? *text : "VISION ERROR: invalid model response";
    }

    std::string map(const std::string& place) const { return Maps::search(place); }
    std::string directions(const std::string& from, const std::string& to) const { return Maps::directions(from,to); }
    std::string osm(const std::string& place) const { return Maps::openStreetMap(place); }

    std::string remember(const std::string& text, const std::string& category="personal") {
        if (!permissions_.check("memory.write")) return "PERMISSION DENIED: memory.write";
        bool ok = memory_.add(text, category); audit_.log("memory.add", ok?"success":"error"); return ok?"MEMORY SAVED":"MEMORY ERROR";
    }
    std::string memorySearch(const std::string& q) const {
        std::ostringstream o; for (const auto& x: memory_.search(q)) o << x << "\n"; return o.str();
    }
    std::string librarySave(const std::string& name, const std::string& content) {
        if (!permissions_.check("library.write")) return "PERMISSION DENIED: library.write";
        bool ok=library_.save(name,content); audit_.log("library.save",ok?name:"error"); return ok?"LIBRARY SAVED: "+name:"LIBRARY ERROR";
    }
    std::string libraryRead(const std::string& name) const { auto s=library_.read(name); return s.empty()?"LIBRARY ERROR: not found":s; }
    std::string librarySearch(const std::string& q) const { std::ostringstream o; for(auto& x:library_.search(q)) o<<x<<"\n"; return o.str(); }
    std::string projectCreate(const std::string& name) { if(!permissions_.check("project.create")) return "PERMISSION DENIED: project.create"; bool ok=projects_.create(name); audit_.log("project.create",ok?name:"error"); return ok?"PROJECT CREATED: "+name:"PROJECT ERROR"; }
    std::string projectList() const { std::ostringstream o; for(auto& x:projects_.list()) o<<x<<"\n"; return o.str(); }

    std::string processRun(const std::string& command) {
        if (!permissions_.check("process.run")) return "PROCESS DENIED: process.run";
#ifdef _WIN32
        FILE* p = _popen(command.c_str(), "r");
#else
        FILE* p = popen(command.c_str(), "r");
#endif
        if (!p) return "PROCESS ERROR: could not start";
        std::string out; char buf[1024]; while (std::fgets(buf,sizeof(buf),p)) out += buf;
#ifdef _WIN32
        int ec = _pclose(p);
#else
        int ec = pclose(p);
#endif
        audit_.log("process.run", std::to_string(ec));
        return "EXIT=" + std::to_string(ec) + "\n" + out;
    }

    void permission(const std::string& key, bool allow) { permissions_.set(key,allow); audit_.log("permission."+key,allow?"allow":"deny"); }
    std::string permissions() const { return permissions_.json(); }
    bool pluginLoad(const std::string& path) { if(!permissions_.check("plugin.load")) return false; bool ok=plugins_.load(path); audit_.log("plugin.load",ok?path:"error"); return ok; }
    void workflowSet(const std::string& event, const std::vector<std::string>& actions) { workflow_.set(event, actions); audit_.log("workflow.set",event); }
    std::string workflowGet(const std::string& event) const { std::ostringstream o; for(auto& x:workflow_.get(event)) o<<x<<"\n"; return o.str(); }
    std::string webSearchUrl(const std::string& q) const { return Search::webUrl(q); }
};

static char* dupC(const std::string& s) {
    char* p = (char*)std::malloc(s.size()+1); if (!p) return nullptr;
    std::memcpy(p,s.data(),s.size()); p[s.size()]='\0'; return p;
}

#ifdef _WIN32
#define CORE_EXPORT extern "C" __declspec(dllexport)
#else
#define CORE_EXPORT extern "C"
#endif

struct CoreHandle { coreai::Core* core = nullptr; };

CORE_EXPORT CoreHandle* core_create() { try { auto* h=new CoreHandle; h->core=new coreai::Core; return h; } catch(...) { return nullptr; } }
CORE_EXPORT void core_destroy(CoreHandle* h) { if(!h)return; delete h->core; delete h; }
CORE_EXPORT void core_free(char* p) { std::free(p); }
CORE_EXPORT char* core_version(CoreHandle* h) { return coreai::dupC(h&&h->core?h->core->version():"CORE ERROR"); }
CORE_EXPORT char* core_status(CoreHandle* h) { return coreai::dupC(h&&h->core?h->core->status():"{\"ok\":false}"); }
CORE_EXPORT char* core_capabilities(CoreHandle* h) { return coreai::dupC(h&&h->core?h->core->capabilities():"{}"); }
CORE_EXPORT char* core_permissions(CoreHandle* h) { return coreai::dupC(h&&h->core?h->core->permissions():"{}"); }
CORE_EXPORT void core_set_model(CoreHandle* h,const char* m) { if(h&&h->core&&m)h->core->setModel(m); }
CORE_EXPORT char* core_chat(CoreHandle* h,const char* p) { return coreai::dupC(h&&h->core&&p?h->core->chat(p):"CORE ERROR"); }
CORE_EXPORT char* core_think(CoreHandle* h,const char* p) { return coreai::dupC(h&&h->core&&p?h->core->think(p):"CORE ERROR"); }
CORE_EXPORT char* core_code(CoreHandle* h,const char* p) { return coreai::dupC(h&&h->core&&p?h->core->code(p):"CORE ERROR"); }
CORE_EXPORT char* core_agent(CoreHandle* h,const char* p) { return coreai::dupC(h&&h->core&&p?h->core->agent(p):"CORE ERROR"); }
CORE_EXPORT char* core_search(CoreHandle* h,const char* q,int n) { return coreai::dupC(h&&h->core&&q?h->core->search(q,n):"CORE ERROR"); }
CORE_EXPORT char* core_image(CoreHandle* h,const char* p,const char* out) { return coreai::dupC(h&&h->core&&p&&out?h->core->image(p,out):"CORE ERROR"); }
CORE_EXPORT char* core_vision(CoreHandle* h,const char* file,const char* q) { return coreai::dupC(h&&h->core&&file&&q?h->core->vision(file,q):"CORE ERROR"); }
CORE_EXPORT char* core_map(CoreHandle* h,const char* q) { return coreai::dupC(h&&h->core&&q?h->core->map(q):"CORE ERROR"); }
CORE_EXPORT char* core_directions(CoreHandle* h,const char* a,const char* b) { return coreai::dupC(h&&h->core&&a&&b?h->core->directions(a,b):"CORE ERROR"); }
CORE_EXPORT char* core_osm(CoreHandle* h,const char* q) { return coreai::dupC(h&&h->core&&q?h->core->osm(q):"CORE ERROR"); }
CORE_EXPORT char* core_remember(CoreHandle* h,const char* t,const char* c) { return coreai::dupC(h&&h->core&&t?h->core->remember(t,c?c:"personal"):"CORE ERROR"); }
CORE_EXPORT char* core_memory_search(CoreHandle* h,const char* q) { return coreai::dupC(h&&h->core&&q?h->core->memorySearch(q):"CORE ERROR"); }
CORE_EXPORT char* core_library_save(CoreHandle* h,const char* n,const char* c) { return coreai::dupC(h&&h->core&&n&&c?h->core->librarySave(n,c):"CORE ERROR"); }
CORE_EXPORT char* core_library_read(CoreHandle* h,const char* n) { return coreai::dupC(h&&h->core&&n?h->core->libraryRead(n):"CORE ERROR"); }
CORE_EXPORT char* core_library_search(CoreHandle* h,const char* q) { return coreai::dupC(h&&h->core&&q?h->core->librarySearch(q):"CORE ERROR"); }
CORE_EXPORT char* core_project_create(CoreHandle* h,const char* n) { return coreai::dupC(h&&h->core&&n?h->core->projectCreate(n):"CORE ERROR"); }
CORE_EXPORT char* core_project_list(CoreHandle* h) { return coreai::dupC(h&&h->core?h->core->projectList():"CORE ERROR"); }
CORE_EXPORT char* core_process_run(CoreHandle* h,const char* c) { return coreai::dupC(h&&h->core&&c?h->core->processRun(c):"CORE ERROR"); }
CORE_EXPORT void core_permission(CoreHandle* h,const char* key,int allow) { if(h&&h->core&&key)h->core->permission(key,allow!=0); }
CORE_EXPORT int core_plugin_load(CoreHandle* h,const char* path) { return h&&h->core&&path&&h->core->pluginLoad(path)?1:0; }
CORE_EXPORT char* core_web_search_url(CoreHandle* h,const char* q) { return coreai::dupC(h&&h->core&&q?h->core->webSearchUrl(q):"CORE ERROR"); }

} // namespace coreai

#ifndef CORE_AI_NO_MAIN

#ifdef _WIN32

namespace coreai_gui {

static constexpr UINT WM_CORE_REPLY = WM_APP + 42;
static constexpr int IDC_CHAT_LOG = 1001;
static constexpr int IDC_INPUT = 1002;
static constexpr int IDC_SEND = 1003;
static constexpr int IDC_STATUS = 1004;

struct GuiState {
    coreai::Core core;
    HWND window = nullptr;
    HWND chatLog = nullptr;
    HWND input = nullptr;
    HWND send = nullptr;
    HWND status = nullptr;
};

static std::wstring utf8ToWide(const std::string& s) {
    if (s.empty()) return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    if (n <= 0) {
        std::wstring out(s.begin(), s.end());
        return out;
    }
    std::wstring out(n, L'\\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), out.data(), n);
    return out;
}

static void setText(HWND h, const std::wstring& s) {
    if (h) SetWindowTextW(h, s.c_str());
}

static void appendChat(HWND h, const std::wstring& s) {
    if (!h) return;
    int len = GetWindowTextLengthW(h);
    SendMessageW(h, EM_SETSEL, (WPARAM)len, (LPARAM)len);
    SendMessageW(h, EM_REPLACESEL, FALSE, (LPARAM)s.c_str());
    SendMessageW(h, EM_SCROLL, SB_BOTTOM, 0);
}

static void updateStatus(GuiState* g, const wchar_t* text) {
    setText(g->status, text ? text : L"");
}

static void sendMessage(GuiState* g) {
    if (!g || !g->input || !g->send) return;
    int len = GetWindowTextLengthW(g->input);
    if (len <= 0) return;

    std::wstring wtext((size_t)len + 1, L'\\0');
    GetWindowTextW(g->input, wtext.data(), len + 1);
    wtext.resize((size_t)len);

    int utf8Size = WideCharToMultiByte(CP_UTF8, 0, wtext.data(), (int)wtext.size(), nullptr, 0, nullptr, nullptr);
    std::string text8;
    if (utf8Size > 0) {
        text8.resize((size_t)utf8Size);
        WideCharToMultiByte(CP_UTF8, 0, wtext.data(), (int)wtext.size(), text8.data(), utf8Size, nullptr, nullptr);
    }
    if (coreai::trim(text8).empty()) return;

    appendChat(g->chatLog, L"You\r\n");
    appendChat(g->chatLog, utf8ToWide(text8));
    appendChat(g->chatLog, L"\r\n\r\nCORE AI\r\n");
    setText(g->input, L"");
    EnableWindow(g->send, FALSE);
    EnableWindow(g->input, FALSE);
    updateStatus(g, L"CORE AI is thinking…");

    std::thread([g, text8]() {
        std::string reply = g->core.chat(text8);
        auto* heap = new std::string(std::move(reply));
        if (!PostMessageW(g->window, WM_CORE_REPLY, 0, (LPARAM)heap)) delete heap;
    }).detach();
}

static LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    GuiState* g = reinterpret_cast<GuiState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    switch (msg) {
        case WM_NCCREATE: {
            auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
            g = reinterpret_cast<GuiState*>(cs->lpCreateParams);
            g->window = hwnd;
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)g);
            return TRUE;
        }
        case WM_CREATE: {
            HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
            g->chatLog = CreateWindowExW(
                WS_EX_CLIENTEDGE, L"EDIT", L"CORE AI\r\nReady. Enter a message below and press Send.\r\n\r\n",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
                0, 0, 0, 0, hwnd, (HMENU)IDC_CHAT_LOG, GetModuleHandleW(nullptr), nullptr);
            g->input = CreateWindowExW(
                WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                0, 0, 0, 0, hwnd, (HMENU)IDC_INPUT, GetModuleHandleW(nullptr), nullptr);
            g->send = CreateWindowExW(
                0, L"BUTTON", L"Send",
                WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                0, 0, 0, 0, hwnd, (HMENU)IDC_SEND, GetModuleHandleW(nullptr), nullptr);
            g->status = CreateWindowExW(
                0, L"STATIC", L"Ready",
                WS_CHILD | WS_VISIBLE,
                0, 0, 0, 0, hwnd, (HMENU)IDC_STATUS, GetModuleHandleW(nullptr), nullptr);

            SendMessageW(g->chatLog, WM_SETFONT, (WPARAM)font, TRUE);
            SendMessageW(g->input, WM_SETFONT, (WPARAM)font, TRUE);
            SendMessageW(g->send, WM_SETFONT, (WPARAM)font, TRUE);
            SendMessageW(g->status, WM_SETFONT, (WPARAM)font, TRUE);

            SetFocus(g->input);
            return 0;
        }
        case WM_SIZE: {
            if (!g) return 0;
            int w = LOWORD(lParam);
            int h = HIWORD(lParam);
            const int margin = 12;
            const int inputH = 32;
            const int buttonW = 92;
            const int statusH = 22;
            MoveWindow(g->chatLog, margin, margin, std::max(0, w - margin*2), std::max(0, h - margin*3 - inputH - statusH), TRUE);
            int y = h - margin - inputH - statusH;
            MoveWindow(g->status, margin, y, std::max(0, w - margin*2), statusH, TRUE);
            y += statusH + 4;
            MoveWindow(g->input, margin, y, std::max(0, w - margin*3 - buttonW), inputH, TRUE);
            MoveWindow(g->send, w - margin - buttonW, y, buttonW, inputH, TRUE);
            return 0;
        }
        case WM_COMMAND:
            if (LOWORD(wParam) == IDC_SEND && HIWORD(wParam) == BN_CLICKED) {
                sendMessage(g);
                return 0;
            }
            if (LOWORD(wParam) == IDC_INPUT && HIWORD(wParam) == EN_CHANGE) return 0;
            break;
        case WM_CORE_REPLY: {
            auto* reply = reinterpret_cast<std::string*>(lParam);
            if (reply) {
                appendChat(g->chatLog, utf8ToWide(*reply));
                appendChat(g->chatLog, L"\r\n\r\n");
                delete reply;
            }
            EnableWindow(g->send, TRUE);
            EnableWindow(g->input, TRUE);
            SetFocus(g->input);
            updateStatus(g, L"Ready");
            return 0;
        }
        case WM_SETFOCUS:
            if (g && g->input) SetFocus(g->input);
            return 0;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

static int run() {
    HINSTANCE instance = GetModuleHandleW(nullptr);
    const wchar_t* className = L"COREAIChatWindowV21";

    WNDCLASSW wc{};
    wc.lpfnWndProc = wndProc;
    wc.hInstance = instance;
    wc.lpszClassName = className;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.hIcon = LoadIconW(nullptr, IDI_APPLICATION);

    if (!RegisterClassW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) return 2;

    auto* state = new GuiState(); // retained for process lifetime so detached replies never outlive the state
    std::wstring title = L"CORE AI 2.2 — Personal AI";
    HWND hwnd = CreateWindowExW(
        0, className, title.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1000, 720,
        nullptr, nullptr, instance, state);
    if (!hwnd) {
        delete state;
        MessageBoxW(nullptr, L"CORE AI could not create its window.", L"CORE AI", MB_ICONERROR);
        return 3;
    }

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    // state is intentionally retained until process exit; detached model workers may still finish after window close.
    return (int)msg.wParam;
}

} // namespace coreai_gui

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
    return coreai_gui::run();
}

#else

static void help() {
    std::cout << "\\nCORE AI 2.2 — PERSONAL AI CORE\\n"
      << "/status\\n/capabilities\\n/permissions\\n/model <name>\\n"
      << "/chat <text>\\n/think <text>\\n/code <text>\\n/agent <task>\\n"
      << "/search <query>\\n/search-url <query>\\n/image <prompt>|<output.png>\\n/vision <image>|<question>\\n"
      << "/map <place>\\n/directions <from>|<to>\\n/osm <place>\\n"
      << "/remember <text>\\n/memory-search <text>\\n/library-save <name>|<content>\\n/library-read <name>\\n/library-search <text>\\n"
      << "/project-create <name>\\n/project-list\\n/process <command>\\n/allow <permission> <0|1>\\n/exit\\n\\n";
}

int main() {
    coreai::Core core;
    std::cout << "============================================\\nCORE AI — PERSONAL AI CORE\\n============================================\\n";
    std::cout << "Version: " << core.version() << "\\nRoot: " << core.root() << "\\n";
    help();
    std::string line;
    while (std::cout << "CORE> " && std::getline(std::cin,line)) {
        line = coreai::trim(line); if(line.empty()) continue;
        if(line=="/exit" || line=="exit" || line=="quit") break;
        if(line=="/help"){help();continue;}
        if(line=="/status"){std::cout<<core.status()<<"\\n";continue;}
        if(line=="/capabilities"){std::cout<<core.capabilities()<<"\\n";continue;}
        if(line=="/permissions"){std::cout<<core.permissions()<<"\\n";continue;}
        if(line.rfind("/model ",0)==0){core.setModel(coreai::trim(line.substr(7)));std::cout<<"Model: "<<core.model()<<"\\n";continue;}
        if(line.rfind("/chat ",0)==0){std::cout<<core.chat(coreai::trim(line.substr(6)))<<"\\n";continue;}
        if(line.rfind("/think ",0)==0){std::cout<<core.think(coreai::trim(line.substr(7)))<<"\\n";continue;}
        if(line.rfind("/code ",0)==0){std::cout<<core.code(coreai::trim(line.substr(6)))<<"\\n";continue;}
        if(line.rfind("/agent ",0)==0){std::cout<<core.agent(coreai::trim(line.substr(7)))<<"\\n";continue;}
        if(line.rfind("/search-url ",0)==0){std::cout<<core.webSearchUrl(coreai::trim(line.substr(12)))<<"\\n";continue;}
        if(line.rfind("/search ",0)==0){std::cout<<core.search(coreai::trim(line.substr(8)))<<"\\n";continue;}
        if(line.rfind("/image ",0)==0){auto p=coreai::splitPipe(line.substr(7));std::cout<<(p.size()>=2?core.image(p[0],p[1]):"Usage: /image prompt|output.png")<<"\\n";continue;}
        if(line.rfind("/vision ",0)==0){auto p=coreai::splitPipe(line.substr(8));std::cout<<(p.size()>=2?core.vision(p[0],p[1]):"Usage: /vision image|question")<<"\\n";continue;}
        if(line.rfind("/map ",0)==0){std::cout<<core.map(coreai::trim(line.substr(5)))<<"\\n";continue;}
        if(line.rfind("/directions ",0)==0){auto p=coreai::splitPipe(line.substr(12));std::cout<<(p.size()>=2?core.directions(p[0],p[1]):"Usage: /directions from|to")<<"\\n";continue;}
        if(line.rfind("/osm ",0)==0){std::cout<<core.osm(coreai::trim(line.substr(5)))<<"\\n";continue;}
        if(line.rfind("/remember ",0)==0){std::cout<<core.remember(coreai::trim(line.substr(10)))<<"\\n";continue;}
        if(line.rfind("/memory-search ",0)==0){std::cout<<core.memorySearch(coreai::trim(line.substr(15)))<<"\\n";continue;}
        if(line.rfind("/library-save ",0)==0){auto p=coreai::splitPipe(line.substr(14));std::cout<<(p.size()>=2?core.librarySave(p[0],p[1]):"Usage: /library-save name|content")<<"\\n";continue;}
        if(line.rfind("/library-read ",0)==0){std::cout<<core.libraryRead(coreai::trim(line.substr(14)))<<"\\n";continue;}
        if(line.rfind("/library-search ",0)==0){std::cout<<core.librarySearch(coreai::trim(line.substr(16)))<<"\\n";continue;}
        if(line.rfind("/project-create ",0)==0){std::cout<<core.projectCreate(coreai::trim(line.substr(16)))<<"\\n";continue;}
        if(line=="/project-list"){std::cout<<core.projectList()<<"\\n";continue;}
        if(line.rfind("/process ",0)==0){std::cout<<core.processRun(coreai::trim(line.substr(9)))<<"\\n";continue;}
        if(line.rfind("/allow ",0)==0){auto raw=coreai::trim(line.substr(7));std::istringstream in(raw);std::string key;int allow=0;in>>key>>allow;if(!key.empty())core.permission(key,allow!=0);std::cout<<core.permissions()<<"\\n";continue;}
        std::cout<<core.chat(line)<<"\\n";
    }
    return 0;
}

#endif
#endif
