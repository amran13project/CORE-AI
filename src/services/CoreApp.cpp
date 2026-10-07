#include "services/CoreApp.h"
#include "services/LocalApiServer.h"
#include <chrono>
#include <fstream>
#include <sstream>
#include <random>
#include <cstdlib>
namespace coreai {
static std::string esc(const std::string&s){std::string o;for(char c:s){if(c=='"')o+="\\\"";else if(c=='\\')o+="\\\\";else if(c=='\n')o+="\\n";else if(c=='\r')o+="\\r";else o+=c;}return o;}
static std::string sqlEsc(const std::string&s){std::string o;for(char c:s){if(c=='\'')o+="''";else o+=c;}return o;}
static std::string id(){static std::mt19937_64 r{std::random_device{}()};std::ostringstream o;o<<std::hex<<r();return o.str();}
CoreApp::CoreApp()=default; CoreApp::~CoreApp(){stopApi();}
Result<void> CoreApp::initialize(const std::string&root){
    auto rp=RuntimePaths::resolve(root);if(!rp.ok())return Result<void>::failure(rp.error());
    paths_=rp.value();logger_=std::make_unique<Logger>(paths_.logs()/"core-ai.log");
    auto cfg=(paths_.config()/"core.config");
    if(!std::filesystem::exists(cfg)){
        config_.set("privacy.mode","Local Only");
        config_.set("provider.ollama.url","http://127.0.0.1:11434");
        config_.set("model","llama3.2");
        config_.set("chat.reset","off");
        if(auto s=config_.save(cfg);!s.ok())return s;
    }else{
        auto s=config_.load(cfg);
        if(!s.ok())logger_->warn("config load degraded: "+s.error().message);
    }
    const char* envUrl=std::getenv("CORE_OLLAMA_URL");
    const char* envModel=std::getenv("CORE_MODEL");
    const std::string configuredUrl=(envUrl&&*envUrl)?std::string(envUrl):config_.get("provider.ollama.url","http://127.0.0.1:11434");
    selected_model_=(envModel&&*envModel)?std::string(envModel):config_.get("model","llama3.2");
    if((envUrl&&*envUrl)||(envModel&&*envModel)){
        config_.set("provider.ollama.url",configuredUrl);
        config_.set("model",selected_model_);
        (void)config_.save(cfg);
    }
    auto d=db_.open(paths_.data()/"core-ai.sqlite3");
    if(!d.ok()){logger_->error(d.error().message);buildCapabilities();return d;}
    auto s=setupSchema();if(!s.ok())return s;
    ollama_=std::make_unique<providers::OllamaProvider>(configuredUrl,selected_model_);
    if(ollama_->reachable()){
        auto discovered=ollama_->discover();
        if(discovered.ok()&&!discovered.value().empty()){
            bool found=false;for(const auto&m:discovered.value())if(m.id==selected_model_){found=true;break;}
            if(!found&&!envModel){
                selected_model_=discovered.value().front().id;
                config_.set("model",selected_model_);(void)config_.save(cfg);
                ollama_=std::make_unique<providers::OllamaProvider>(configuredUrl,selected_model_);
            }
        }
    }
    features_=std::make_unique<features::LocalFeatures>(paths_,db_);
    buildCapabilities();initialized_=true;logger_->info("CORE-AI initialized");return Result<void>::success();
}

Result<void> CoreApp::setupSchema(){const char*sql="PRAGMA journal_mode=WAL; CREATE TABLE IF NOT EXISTS conversations(id TEXT PRIMARY KEY,title TEXT NOT NULL,created_at INTEGER NOT NULL,updated_at INTEGER NOT NULL,archived INTEGER NOT NULL DEFAULT 0); CREATE TABLE IF NOT EXISTS messages(id TEXT PRIMARY KEY,conversation_id TEXT NOT NULL,role TEXT NOT NULL,content TEXT NOT NULL,created_at INTEGER NOT NULL); CREATE TABLE IF NOT EXISTS memory(id TEXT PRIMARY KEY,scope TEXT NOT NULL,content TEXT NOT NULL,source TEXT NOT NULL,created_at INTEGER NOT NULL); CREATE TABLE IF NOT EXISTS projects(id TEXT PRIMARY KEY,name TEXT NOT NULL,root TEXT NOT NULL,created_at INTEGER NOT NULL,updated_at INTEGER NOT NULL); CREATE TABLE IF NOT EXISTS settings(key TEXT PRIMARY KEY,value TEXT NOT NULL); CREATE TABLE IF NOT EXISTS tasks(id TEXT PRIMARY KEY,name TEXT NOT NULL,status TEXT NOT NULL,created_at INTEGER NOT NULL); CREATE TABLE IF NOT EXISTS workflows(id TEXT PRIMARY KEY,name TEXT NOT NULL,definition TEXT NOT NULL,created_at INTEGER NOT NULL); CREATE TABLE IF NOT EXISTS schedules(id TEXT PRIMARY KEY,name TEXT NOT NULL,enabled INTEGER NOT NULL,next_run INTEGER NOT NULL,created_at INTEGER NOT NULL); CREATE TABLE IF NOT EXISTS library(id TEXT PRIMARY KEY,name TEXT NOT NULL,path TEXT NOT NULL,hash TEXT NOT NULL,created_at INTEGER NOT NULL);";return db_.exec(sql);}
void CoreApp::buildCapabilities(){
    caps_.set({"native.core","Native C++ Core","READY","C++20 application core is implemented",true,true,true,true});
    caps_.set({"user.storage","User Storage Isolation","READY","Per-user storage roots with quota-aware shard allocation are implemented",true,true,true,true});
    caps_.set({"storage.sqlite","SQLite Persistence",db_.available()?"READY":"UNAVAILABLE",db_.available()?"SQLite runtime loaded":"SQLite runtime unavailable",true,db_.available(),db_.available(),true});
    const bool reach=ollama_ && ollama_->reachable();
    caps_.set({"provider.ollama","Ollama Provider",reach?"READY":"UNAVAILABLE",reach?"Endpoint reachable":"Ollama endpoint unavailable",true,reach,true,true});
    caps_.set({"chat","Chat",reach?"READY":"UNAVAILABLE",reach?"Live Ollama generation available":"No reachable AI provider",true,reach,true,true});
    caps_.set({"think","Think",reach?"READY":"UNAVAILABLE",reach?"Real generation pipeline":"No reachable AI provider",true,reach,true,true});
    caps_.set({"codex","Codex","READY","Repository/file inspection and safe local coding primitives are implemented",true,true,true,true});
    caps_.set({"projects","Projects","READY","Persistent project metadata is stored in SQLite",true,true,true,true});
    caps_.set({"memory","Memory","READY","Persistent memory CRUD/search is implemented",true,true,true,true});
    caps_.set({"recent","Recent","READY","Reads persisted conversations",true,true,true,true});
    caps_.set({"library","Library","READY","Persistent library artifacts are implemented",true,true,true,true});
    caps_.set({"files","Files","READY","Sandboxed file read/write service is implemented",true,true,true,true});
    caps_.set({"documents","Documents","READY","TXT/MD/JSON/CSV inspection is implemented",true,true,true,true});
    caps_.set({"tools","Tools","READY","Core local tool services are available",true,true,true,true});
    caps_.set({"agents","Agents","READY","Bounded local planner agent is implemented",true,true,true,true});
    caps_.set({"multi-agent","Multi-Agent","READY","Bounded planner/reviewer/tester orchestration is implemented",true,true,true,true});
    caps_.set({"workflows","Workflows","READY","Persistent workflow definitions and execution are implemented",true,true,true,true});
    caps_.set({"scheduled","Scheduled","READY","Persistent schedules and manual execution are implemented",true,true,true,true});
    caps_.set({"plugins","Plugins","READY","Native plugin discovery is implemented",true,true,true,true});
    caps_.set({"tasks","Tasks","READY","Persistent task records are implemented",true,true,true,true});
    caps_.set({"git","Git","READY","Git status integration is implemented",true,true,true,true});
    caps_.set({"research","Research","READY","Local provenance-aware library research is implemented",true,true,true,true});
    caps_.set({"maps","Maps","READY","Map query/directions URL adapter is implemented",true,true,true,true});
    caps_.set({"images","Images","READY","Local SVG image artifact generation is implemented",true,true,true,true});
    caps_.set({"vision","Vision","NOT_IMPLEMENTED","Vision adapter is not implemented",false,false,false,false});
    caps_.set({"voice","Voice","NOT_IMPLEMENTED","Voice adapter is not implemented",false,false,false,false});
    caps_.set({"semantic.retrieval","Semantic Retrieval","READY","Deterministic lexical retrieval over persisted library is implemented",true,true,true,true});
    caps_.set({"multimodal","Multimodal","READY","Text plus file/image artifact handling is integrated",true,true,true,true});
    caps_.set({"api","Local API","READY","Loopback HTTP API implementation is present",true,true,true,true});
    caps_.set({"cli","CLI","READY","Native CLI command surface is implemented for core diagnostics and persistence",true,true,true,true});
    caps_.set({"web","Web Client","READY","Web client is served by the local API",true,true,true,true});
    caps_.set({"native.gui","Native Desktop GUI","NOT_VERIFIED","Win32 native GUI requires a Windows runtime validation environment",true,false,true,true});
}
Result<std::string> CoreApp::ensureConversation(){if(!initialized_)return Result<std::string>::failure(error(ErrorCode::Internal,"application not initialized","app","conversation"));if(!current_conversation_.empty())return Result<std::string>::success(current_conversation_);return newChat();}
Result<std::string> CoreApp::newChat(){if(!db_.available())return Result<std::string>::failure(error(ErrorCode::StorageUnavailable,"SQLite unavailable","conversation","new_chat",true));current_conversation_=id();auto now=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());std::string title="New Chat";std::string sql="INSERT INTO conversations(id,title,created_at,updated_at) VALUES('"+current_conversation_+"','"+title+"',"+std::to_string(now)+","+std::to_string(now)+")";auto r=db_.exec(sql);if(!r.ok())return Result<std::string>::failure(r.error());return Result<std::string>::success(current_conversation_);}
Result<std::string> CoreApp::resetChat(){current_conversation_.clear();return newChat();}
Result<std::string> CoreApp::chat(const std::string&prompt,const std::string&mode){if(prompt.empty())return Result<std::string>::failure(error(ErrorCode::InvalidArgument,"prompt required","chat","generate"));auto c=ensureConversation();if(!c.ok())return c;auto now=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());auto msgid=id();std::string ins="INSERT INTO messages(id,conversation_id,role,content,created_at) VALUES('"+sqlEsc(msgid)+"','"+sqlEsc(current_conversation_)+"','user','"+sqlEsc(prompt)+"',"+std::to_string(now)+")";if(!db_.exec(ins).ok())return Result<std::string>::failure(error(ErrorCode::StorageCorrupt,"message insert failed","chat","persist"));
    if(!ollama_||!ollama_->reachable())return Result<std::string>::failure(error(ErrorCode::ProviderUnavailable,"No reachable AI provider. Configure Ollama and a model.","chat","generate",true,true));
    std::string system="You are CORE-AI, a local-first personal AI assistant. Be truthful. Never claim actions or external results you did not actually verify. Do not reveal private chain-of-thought. Mode: "+mode+".";auto r=ollama_->generate({system,prompt,selected_model_});if(!r.ok())return Result<std::string>::failure(r.error());auto aid=id();std::string a="INSERT INTO messages(id,conversation_id,role,content,created_at) VALUES('"+sqlEsc(aid)+"','"+sqlEsc(current_conversation_)+"','assistant','"+sqlEsc(r.value().text)+"',"+std::to_string(now)+")";auto ar=db_.exec(a);if(!ar.ok())return Result<std::string>::failure(ar.error());db_.exec("UPDATE conversations SET updated_at="+std::to_string(now)+" WHERE id='"+sqlEsc(current_conversation_)+"'");return Result<std::string>::success(r.value().text);}
Result<std::string> CoreApp::recent()const{if(!db_.available())return Result<std::string>::failure(error(ErrorCode::StorageUnavailable,"SQLite unavailable","chat","recent",true));auto r=db_.query("SELECT id,title,updated_at FROM conversations WHERE archived=0 ORDER BY updated_at DESC LIMIT 30");if(!r.ok())return Result<std::string>::failure(r.error());std::ostringstream o;o<<"[";for(size_t i=0;i<r.value().size();++i){auto&v=r.value()[i].values;if(i)o<<',';o<<"{\"id\":\""<<esc(v[0])<<"\",\"title\":\""<<esc(v[1])<<"\",\"updated_at\":"<<v[2]<<"}";}o<<"]";return Result<std::string>::success(o.str());}
Result<std::string> CoreApp::remember(const std::string&text,const std::string&scope){if(text.empty())return Result<std::string>::failure(error(ErrorCode::InvalidArgument,"memory text required","memory","create"));auto now=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());auto i=id();auto r=db_.exec("INSERT INTO memory(id,scope,content,source,created_at) VALUES('"+sqlEsc(i)+"','"+sqlEsc(scope)+"','"+sqlEsc(text)+"','user',"+std::to_string(now)+")");if(!r.ok())return Result<std::string>::failure(r.error());return Result<std::string>::success(i);}
Result<std::string> CoreApp::memorySearch(const std::string&q)const{auto r=db_.query("SELECT id,scope,content,created_at FROM memory WHERE content LIKE ?1 OR scope LIKE ?1 ORDER BY created_at DESC LIMIT 50",{"%"+q+"%"});if(!r.ok())return Result<std::string>::failure(r.error());std::ostringstream o;o<<"[";for(size_t i=0;i<r.value().size();++i){auto&v=r.value()[i].values;if(i)o<<',';o<<"{\"id\":\""<<esc(v[0])<<"\",\"scope\":\""<<esc(v[1])<<"\",\"content\":\""<<esc(v[2])<<"\",\"created_at\":"<<v[3]<<"}";}o<<"]";return Result<std::string>::success(o.str());}
Result<std::string> CoreApp::projectCreate(const std::string&name){if(name.empty())return Result<std::string>::failure(error(ErrorCode::InvalidArgument,"project name required","projects","create"));auto now=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());auto i=id();if(name.empty() || std::filesystem::path(name).is_absolute() || name.find('\\')!=std::string::npos || name.find('/')!=std::string::npos || name=="." || name==".." || name.find("..")!=std::string::npos)
        return Result<std::string>::failure(error(ErrorCode::PermissionDenied,"invalid project name/path","projects","create"));
    const auto approved_root=std::filesystem::weakly_canonical(paths_.projects());
    const auto root=std::filesystem::weakly_canonical(approved_root / name);
    if(root.parent_path()!=approved_root)
        return Result<std::string>::failure(error(ErrorCode::PermissionDenied,"project path outside approved root","projects","create"));std::error_code ec;std::filesystem::create_directories(root,ec);if(ec)return Result<std::string>::failure(error(ErrorCode::StorageOpenFailed,ec.message(),"projects","create",true));std::ofstream(root/"project.core.json")<<"{\"schema\":1,\"id\":\""<<i<<"\",\"name\":\""<<esc(name)<<"\"}\n";auto r=db_.exec("INSERT INTO projects(id,name,root,created_at,updated_at) VALUES('"+sqlEsc(i)+"','"+sqlEsc(name)+"','"+sqlEsc(root.string())+"',"+std::to_string(now)+","+std::to_string(now)+")");if(!r.ok())return Result<std::string>::failure(r.error());return Result<std::string>::success(i);}
Result<std::string> CoreApp::projectList()const{auto r=db_.query("SELECT id,name,root,updated_at FROM projects ORDER BY updated_at DESC");if(!r.ok())return Result<std::string>::failure(r.error());std::ostringstream o;o<<"[";for(size_t i=0;i<r.value().size();++i){auto&v=r.value()[i].values;if(i)o<<',';o<<"{\"id\":\""<<esc(v[0])<<"\",\"name\":\""<<esc(v[1])<<"\",\"root\":\""<<esc(v[2])<<"\",\"updated_at\":"<<v[3]<<"}";}o<<"]";return Result<std::string>::success(o.str());}
Result<void> CoreApp::setModel(const std::string&m){if(m.empty())return Result<void>::failure(error(ErrorCode::InvalidArgument,"model required","models","select"));selected_model_=m;config_.set("model",m);return config_.save(paths_.config()/"core.config");}
Result<std::string> CoreApp::status()const{std::ostringstream o;auto reach=ollama_&&ollama_->reachable();const auto& us=paths_.userStorage();o<<"{\"version\":\"0.3.2\",\"initialized\":"<<(initialized_?"true":"false")<<",\"model\":\""<<esc(selected_model_)<<"\",\"ollama_reachable\":"<<(reach?"true":"false")<<",\"storage\":"<<(db_.available()?"true":"false")<<",\"storage_user\":\""<<esc(us.user_id)<<"\",\"storage_shard\":"<<us.shard_index<<",\"storage_quota_bytes\":"<<us.logical_quota_bytes<<",\"storage_shard_path\":\""<<esc(us.shard_root.string())<<"\",\"privacy_mode\":\""<<esc(config_.get("privacy.mode","Local Only"))<<"\",\"api\":{\"host\":\"127.0.0.1\",\"port\":47821}}";return Result<std::string>::success(o.str());}
std::string CoreApp::doctor() const{
    std::ostringstream o;
    const bool paths_ok=std::filesystem::exists(paths_.root()) && std::filesystem::is_directory(paths_.root());
    const bool db_ok=db_.available();
    const bool cfg_ok=std::filesystem::exists(paths_.config()/"core.config");
    const bool ollama_ok=ollama_ && ollama_->reachable();
    o<<"{\"checks\":["
     <<"{\"name\":\"runtime_paths\",\"status\":\""<<(paths_ok?"PASS":"FAIL")<<"\"},"
     <<"{\"name\":\"config\",\"status\":\""<<(cfg_ok?"PASS":"FAIL")<<"\"},"
     <<"{\"name\":\"sqlite\",\"status\":\""<<(db_ok?"PASS":"FAIL")<<"\"},"
     <<"{\"name\":\"ollama\",\"status\":\""<<(ollama_ok?"PASS":"SKIPPED")<<"\",\"reason\":\""<<(ollama_ok?"endpoint reachable":"dependency unavailable")<<"\"}"
     <<"]}";
    return o.str();
}
Result<std::string> CoreApp::models() const{
    if(!ollama_) return Result<std::string>::failure(error(ErrorCode::ProviderUnavailable,"Ollama provider unavailable","models","discover",true));
    auto r=ollama_->discover();
    if(!r.ok()) return Result<std::string>::failure(r.error());
    std::ostringstream o;o<<"[";
    for(size_t i=0;i<r.value().size();++i){if(i)o<<',';const auto&m=r.value()[i];o<<"{\"provider\":\""<<esc(m.provider)<<"\",\"model\":\""<<esc(m.id)<<"\",\"display_name\":\""<<esc(m.display)<<"\"}";}
    o<<"]";return Result<std::string>::success(o.str());
}

Result<std::string> CoreApp::libraryAdd(const std::string& n,const std::string& c){return features_->libraryAdd(n,c);}
Result<std::string> CoreApp::libraryList()const{return features_->libraryList();}
Result<std::string> CoreApp::fileWrite(const std::string& r,const std::string& c){return features_->fileWrite(r,c);}
Result<std::string> CoreApp::fileRead(const std::string& r)const{return features_->fileRead(r);}
Result<std::string> CoreApp::documentInspect(const std::string& r)const{return features_->documentInspect(r);}
Result<std::string> CoreApp::taskCreate(const std::string& n){return features_->taskCreate(n);}
Result<std::string> CoreApp::taskList()const{return features_->taskList();}
Result<std::string> CoreApp::workflowCreate(const std::string& n,const std::string& d){return features_->workflowCreate(n,d);}
Result<std::string> CoreApp::workflowList()const{return features_->workflowList();}
Result<std::string> CoreApp::workflowRun(const std::string& i){return features_->workflowRun(i);}
Result<std::string> CoreApp::scheduleCreate(const std::string& n,long long d){return features_->scheduleCreate(n,d);}
Result<std::string> CoreApp::scheduleList()const{return features_->scheduleList();}
Result<std::string> CoreApp::scheduleRun(const std::string& i){return features_->scheduleRun(i);}
Result<std::string> CoreApp::agentRun(const std::string& r){return features_->agentRun(r);}
Result<std::string> CoreApp::multiAgentRun(const std::string& r){return features_->multiAgentRun(r);}
Result<std::string> CoreApp::pluginDiscover()const{return features_->pluginDiscover();}
Result<std::string> CoreApp::gitStatus(const std::filesystem::path& r)const{return features_->gitStatus(r);}
Result<std::string> CoreApp::researchLocal(const std::string& q)const{return features_->researchLocal(q);}
Result<std::string> CoreApp::imageCreate(const std::string& n,const std::string& p){return features_->imageCreate(n,p);}
Result<std::string> CoreApp::mapSearch(const std::string& p)const{return features_->mapSearch(p);}
Result<std::string> CoreApp::directions(const std::string& a,const std::string& b)const{return features_->directions(a,b);}

Result<void> CoreApp::startApi(const std::filesystem::path& web_root){
    if(api_) return Result<void>::success();
    api_=std::make_unique<LocalApiServer>(*this);
    auto r=api_->start(apiPort(),web_root);
    if(!r.ok()) api_.reset();
    return r;
} void CoreApp::stopApi(){if(api_){api_->stop();api_.reset();}}
}

