#include "agents/AgentManager.h"
#include <sstream>
namespace core::agents {
AgentManager::AgentManager(ai::ModelRouter& m,tools::ToolRegistry& t,security::PermissionManager& p,audit::AuditLog& a):models_(m),tools_(t),permissions_(p),audit_(a){}
std::string AgentManager::run(const std::string& request,const std::string& workingDirectory){
    std::ostringstream out; out << "PLAN\n";
    for(auto& s:planner_.create(request)) out<<s.index<<". "<<s.action<<" — "<<s.detail<<'\n';
    if(auto* model=models_.select(true)){
        ai::AIRequest q;
        q.system="You are CORE AI planning assistant. Give concise, actionable engineering guidance. Do not claim actions were executed.\n";
        q.prompt=request;
        auto r=model->complete(q);
        audit_.record("agent","model.plan",r.ok?"success":"failed");
        if(r.ok) out<<"\nLOCAL MODEL\n"<<r.text;
        else out<<"\nLOCAL MODEL UNAVAILABLE\n"<<r.error;
    } else out<<"\nLOCAL MODEL: not configured\n";
    return out.str();
}
}
