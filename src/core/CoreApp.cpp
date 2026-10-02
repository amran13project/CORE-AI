#include "core/CoreApp.h"
#include "ai/OllamaProvider.h"
#include "tools/FileTool.h"
#include "tools/ProcessTool.h"
#include <filesystem>
#include <sstream>
#include <cstdlib>

namespace core {
CoreApp::CoreApp():logger_("core.log"),memory_("memory.db.txt"),audit_("audit.log"),plugins_(*this){}
bool CoreApp::initialize(const std::string& root){
    root_=std::filesystem::absolute(root).string();
    std::filesystem::create_directories(std::filesystem::path(root_)/".core");
    config_.load((std::filesystem::path(root_)/"core.config").string());
    if(config_.get("model").empty()) config_.set("model", std::getenv("CORE_MODEL")?std::getenv("CORE_MODEL"):"llama3.2");
    if(config_.get("model").empty()) return false;
    models_.add(std::make_unique<ai::OllamaProvider>(config_.get("model","llama3.2")));
    tools_.add(std::make_unique<tools::FileTool>());
    tools_.add(std::make_unique<tools::ProcessTool>());
    agents_=std::make_unique<agents::AgentManager>(models_,tools_,permissions_,audit_);
    logger_.info("CORE initialized at " + root_);
    return true;
}
std::string CoreApp::chat(const std::string& prompt){
    if(prompt.empty()) return "Please enter a prompt.";
    auto* model=models_.select(true); if(!model) return "No model provider configured.";
    const auto memories=memory_.search(prompt,5);
    std::ostringstream system; system<<"You are CORE AI, a local-first personal AI. Be helpful and factual. Never claim an action was executed unless a tool result proves it.\n";
    if(!memories.empty()){system<<"Relevant memory:\n";for(const auto&m:memories)system<<"- ["<<m.scope<<"] "<<m.text<<"\n";}
    auto result=model->complete({system.str(),prompt});
    audit_.record("user","chat",result.ok?"success":"failed");
    return result.ok?result.text:("MODEL ERROR: "+result.error+"\n"+result.text);
}
void CoreApp::addMemory(const std::string& scope,const std::string& text){memory_.add(scope,text);audit_.record("user","memory.add","success");}
std::string CoreApp::searchMemory(const std::string& query) const { std::ostringstream out; for(const auto&m:memory_.search(query)){out<<m.id<<" ["<<m.scope<<"] "<<m.text<<'\n';} return out.str().empty()?"No memory found.":out.str(); }
std::string CoreApp::runAgent(const std::string& request){audit_.record("user","agent.run","started");return agents_?agents_->run(request,root_):"Agent manager unavailable.";}
std::string CoreApp::tool(const std::string& id,const std::string& input,const std::string& cwd){
    if(id=="process"&&!permissions_.allows("process.execute")) return "DENIED: process.execute permission is not granted.";
    if(id=="file"&&input.rfind("write|",0)==0&&!permissions_.allows("file.write")) return "DENIED: file.write permission is not granted.";
    auto r=tools_.invoke(id,input,{cwd.empty()?root_:cwd}); audit_.record("tool",id,r.ok?"success":"failed");
    return (r.ok?"OK":"FAIL")+std::string(" [exit=")+std::to_string(r.exitCode)+"]\n"+r.output;
}
std::string CoreApp::doctor() const {
    std::ostringstream out; out<<"CORE AI DOCTOR\n";
    out<<"Root: "<<root_<<"\n";
    out<<"Model providers: "; for(const auto&id:models_.providerIds())out<<id<<' '; out<<"\n";
    out<<"Tools: "; for(const auto&id:tools_.ids())out<<id<<' '; out<<"\n";
    out<<"Permissions: memory.read="<<permissions_.allows("memory.read")<<", file.read="<<permissions_.allows("file.read")<<", file.write="<<permissions_.allows("file.write")<<", process.execute="<<permissions_.allows("process.execute")<<"\n";
    return out.str();
}
std::string CoreApp::capabilities() const {
    return "IMPLEMENTED: native core, local-model adapter, chat, memory, files, process tool, agents/planner, verification records, audit log, project data, workflow registry, plugin loader, console UI";
}
}
