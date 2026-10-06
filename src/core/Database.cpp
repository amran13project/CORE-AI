#include "core/Database.h"
#include <cstring>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dlfcn.h>
#endif
struct sqlite3{}; struct sqlite3_stmt{};
namespace coreai {
struct Database::Api {
    int (*open)(const char*,sqlite3**)=nullptr; int (*close)(sqlite3*)=nullptr; int (*exec)(sqlite3*,const char*,int(*)(void*,int,char**,char**),void*,char**)=nullptr;
    int (*prepare)(sqlite3*,const char*,int,sqlite3_stmt**,const char**)=nullptr; int (*bind_text)(sqlite3_stmt*,int,const char*,int,void(*)(void*))=nullptr;
    int (*step)(sqlite3_stmt*)=nullptr; const unsigned char* (*column_text)(sqlite3_stmt*,int)=nullptr; int (*column_count)(sqlite3_stmt*)=nullptr;
    int (*finalize)(sqlite3_stmt*)=nullptr; const char* (*errmsg)(sqlite3*)=nullptr;
};
static constexpr int SQLITE_OK=0,SQLITE_ROW=100,SQLITE_DONE=101,SQLITE_TRANSIENT=-1;
Database::~Database(){close();}
static void unload(void* m){if(!m)return;
#ifdef _WIN32
FreeLibrary((HMODULE)m);
#else
dlclose(m);
#endif
}
static void* loadSym(void*m,const char*n){
#ifdef _WIN32
return (void*)GetProcAddress((HMODULE)m,n);
#else
return dlsym(m,n);
#endif
}
Result<void> Database::open(const std::filesystem::path&p){close();
#ifdef _WIN32
    // Prefer the application/test directory so a packaged CORE-AI never depends on
    // a machine-wide SQLite installation. Fall back to normal DLL search for dev setups.
    char module_path[MAX_PATH]{};
    const DWORD module_len=GetModuleFileNameA(nullptr,module_path,MAX_PATH);
    if(module_len>0 && module_len<MAX_PATH){
        std::filesystem::path exe_dir=std::filesystem::path(module_path).parent_path();
        for(const auto& candidate : {exe_dir/"sqlite3.dll", exe_dir/"sqlite3_x64.dll"}){
            if(std::filesystem::exists(candidate)){
                module_=(void*)LoadLibraryA(candidate.string().c_str());
                if(module_) break;
            }
        }
    }
    if(!module_){
        const char* names[]={"sqlite3.dll","sqlite3_x64.dll"};
        for(auto n:names){module_=(void*)LoadLibraryA(n);if(module_)break;}
    }
#else
    const char* names[]={"libsqlite3.so.0","libsqlite3.so"}; for(auto n:names){module_=dlopen(n,RTLD_NOW);if(module_)break;}
#endif
    if(!module_)return Result<void>::failure(error(ErrorCode::StorageUnavailable,"SQLite runtime library not found","storage","load_sqlite",true));
    api_=new Api();
#define LOAD(name,field) api_->field=reinterpret_cast<decltype(api_->field)>(loadSym(module_,name))
    LOAD("sqlite3_open",open); LOAD("sqlite3_close",close); LOAD("sqlite3_exec",exec); LOAD("sqlite3_prepare_v2",prepare); LOAD("sqlite3_bind_text",bind_text); LOAD("sqlite3_step",step); LOAD("sqlite3_column_text",column_text); LOAD("sqlite3_column_count",column_count); LOAD("sqlite3_finalize",finalize); LOAD("sqlite3_errmsg",errmsg);
#undef LOAD
    if(!api_->open||!api_->close||!api_->exec||!api_->prepare||!api_->bind_text||!api_->step||!api_->column_text||!api_->column_count||!api_->finalize||!api_->errmsg){close();return Result<void>::failure(error(ErrorCode::StorageUnavailable,"SQLite API incomplete","storage","load_sqlite",true));}
    if(api_->open(p.string().c_str(),&db_)!=SQLITE_OK){last_error_=api_->errmsg(db_?db_:nullptr);close();return Result<void>::failure(error(ErrorCode::StorageOpenFailed,last_error_,"storage","open",true));}
    return Result<void>::success();
}
void Database::close(){if(api_&&db_)api_->close(db_);db_=nullptr;if(module_)unload(module_);module_=nullptr;delete api_;api_=nullptr;}
Result<void> Database::exec(const std::string&sql){if(!db_)return Result<void>::failure(error(ErrorCode::StorageUnavailable,"database not open","storage","exec",true));char* err=nullptr;int rc=api_->exec(db_,sql.c_str(),nullptr,nullptr,&err);if(rc!=SQLITE_OK){last_error_=err?err:"SQLite error";return Result<void>::failure(error(ErrorCode::StorageCorrupt,last_error_,"storage","exec",true));}return Result<void>::success();}
Result<std::vector<Row>> Database::query(const std::string&sql,const std::vector<std::string>&params) const{if(!db_)return Result<std::vector<Row>>::failure(error(ErrorCode::StorageUnavailable,"database not open","storage","query",true));sqlite3_stmt* st=nullptr;const char* tail=nullptr;int rc=api_->prepare(db_,sql.c_str(),-1,&st,&tail);if(rc!=SQLITE_OK)return Result<std::vector<Row>>::failure(error(ErrorCode::StorageCorrupt,api_->errmsg(db_),"storage","prepare"));for(size_t i=0;i<params.size();++i){if(api_->bind_text(st,(int)i+1,params[i].c_str(),(int)params[i].size(),reinterpret_cast<void(*)(void*)>(SQLITE_TRANSIENT))!=SQLITE_OK){api_->finalize(st);return Result<std::vector<Row>>::failure(error(ErrorCode::StorageCorrupt,api_->errmsg(db_),"storage","bind"));}}
std::vector<Row> rows;while((rc=api_->step(st))==SQLITE_ROW){Row r;int n=api_->column_count(st);for(int i=0;i<n;++i){const unsigned char* t=api_->column_text(st,i);r.values.emplace_back(t?(const char*)t:"");}rows.push_back(std::move(r));}api_->finalize(st);if(rc!=SQLITE_DONE)return Result<std::vector<Row>>::failure(error(ErrorCode::StorageCorrupt,api_->errmsg(db_),"storage","step"));return Result<std::vector<Row>>::success(std::move(rows));}
}
