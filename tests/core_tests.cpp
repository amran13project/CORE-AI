#include "services/CoreApp.h"
#include <filesystem>
#include <fstream>
#include "core/UserStorage.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

static int failures = 0;
static void pass(const char* n) { std::cout << "PASS " << n << std::endl; }
static void fail(const char* n, const std::string& why) { std::cerr << "FAIL " << n << ": " << why << std::endl; ++failures; }
#define CHECK_TRUE(expr, name) do { if ((expr)) pass(name); else fail(name, #expr); } while(0)
static bool okText(const coreai::Result<std::string>& r) { return r.ok() && !r.value().empty(); }

int main() {
    const auto root = std::filesystem::temp_directory_path() / "core-ai-full-pass-test";
    std::error_code ec;
    std::filesystem::remove_all(root, ec);

    {
    coreai::CoreApp app;
    auto init = app.initialize(root.string());
    if(!init.ok()) fail("initialize", init.error().message); else pass("initialize");
    if(!app.db().available()) fail("sqlite", app.db().lastError().empty()?"SQLite database not available":app.db().lastError()); else pass("sqlite");
    if (!init.ok() || !app.db().available()) return 1;

    auto c = app.newChat();
    CHECK_TRUE(okText(c), "new-chat");
    auto r = app.recent();
    CHECK_TRUE(okText(r), "recent");
    if (c.ok() && r.ok()) CHECK_TRUE(r.value().find(c.value()) != std::string::npos, "recent-contains-chat");

    auto reset = app.resetChat();
    CHECK_TRUE(okText(reset), "reset-chat");
    if (c.ok() && reset.ok()) CHECK_TRUE(reset.value() != c.value(), "reset-preserves-history");

    auto mem = app.remember("CORE full-product memory", "personal");
    CHECK_TRUE(okText(mem), "memory-write");
    auto ms = app.memorySearch("full-product");
    CHECK_TRUE(okText(ms), "memory-search");
    if (ms.ok()) CHECK_TRUE(ms.value().find("CORE full-product memory") != std::string::npos, "memory-persistence-query");

    auto proj = app.projectCreate("Demo");
    CHECK_TRUE(okText(proj), "project-create");
    auto pl = app.projectList();
    CHECK_TRUE(okText(pl), "project-list");
    if (pl.ok()) CHECK_TRUE(pl.value().find("Demo") != std::string::npos, "project-persistence");
    auto bad = app.projectCreate("../escape");
    CHECK_TRUE(!bad.ok(), "project-traversal-blocked");

    auto fw = app.fileWrite("docs/test.txt", "document content");
    CHECK_TRUE(okText(fw), "file-write");
    auto fr = app.fileRead("docs/test.txt");
    CHECK_TRUE(okText(fr), "file-read");
    if (fr.ok()) CHECK_TRUE(fr.value() == "document content", "file-roundtrip");

    const auto sample = root / "temp" / "docs" / "sample.md";
    std::filesystem::create_directories(sample.parent_path(), ec);
    std::ofstream(sample) << "hello";
    auto di = app.documentInspect("docs/sample.md");
    CHECK_TRUE(okText(di), "document-inspect");
    if (di.ok()) CHECK_TRUE(di.value().find("sample.md") != std::string::npos, "document-parser");

    auto fb = app.fileRead("../config/core.config");
    CHECK_TRUE(!fb.ok(), "file-traversal-blocked");

    auto lib = app.libraryAdd("reference.md", "CORE reference library");
    CHECK_TRUE(okText(lib), "library-add");
    auto libs = app.libraryList();
    CHECK_TRUE(okText(libs), "library-list");
    if (libs.ok()) CHECK_TRUE(libs.value().find("reference.md") != std::string::npos, "library-persistence");

    auto research = app.researchLocal("reference");
    CHECK_TRUE(okText(research), "research-local");
    if (research.ok()) CHECK_TRUE(research.value().find("reference.md") != std::string::npos, "research-provenance");

    auto task = app.taskCreate("full-pass-task");
    CHECK_TRUE(okText(task), "task-create");
    auto tasks = app.taskList();
    CHECK_TRUE(okText(tasks), "task-list");
    if (task.ok() && tasks.ok()) CHECK_TRUE(tasks.value().find(task.value()) != std::string::npos, "task-persistence");

    auto wf = app.workflowCreate("full-pass-workflow", "[{\"type\":\"transform\"}]");
    CHECK_TRUE(okText(wf), "workflow-create");
    auto wfl = app.workflowList();
    CHECK_TRUE(okText(wfl), "workflow-list");
    if (wf.ok() && wfl.ok()) CHECK_TRUE(wfl.value().find(wf.value()) != std::string::npos, "workflow-persistence");
    auto wr = wf.ok() ? app.workflowRun(wf.value()) : coreai::Result<std::string>::failure(coreai::error(coreai::ErrorCode::Internal, "workflow creation failed", "test", "workflow-run"));
    CHECK_TRUE(okText(wr), "workflow-run");

    auto sch = app.scheduleCreate("full-pass-schedule", 0);
    CHECK_TRUE(okText(sch), "schedule-create");
    auto sl = app.scheduleList();
    CHECK_TRUE(okText(sl), "schedule-list");
    if (sch.ok() && sl.ok()) CHECK_TRUE(sl.value().find(sch.value()) != std::string::npos, "schedule-persistence");
    auto sr = sch.ok() ? app.scheduleRun(sch.value()) : coreai::Result<std::string>::failure(coreai::error(coreai::ErrorCode::Internal, "schedule creation failed", "test", "schedule-run"));
    CHECK_TRUE(okText(sr), "schedule-run");

    auto ar = app.agentRun("inspect and verify local project");
    CHECK_TRUE(okText(ar), "agent-run");
    auto mar = app.multiAgentRun("review local project");
    CHECK_TRUE(okText(mar), "multi-agent-run");
    auto pd = app.pluginDiscover();
    CHECK_TRUE(pd.ok(), "plugin-discovery");

    auto image = app.imageCreate("test-image", "CORE local image artifact");
    CHECK_TRUE(okText(image), "image-artifact");

    auto caps = app.capabilities();
    CHECK_TRUE(caps.find("\"codex\"") != std::string::npos, "capability-registry");
    auto status = app.status();
    CHECK_TRUE(okText(status), "status");
    if (status.ok()) CHECK_TRUE(status.value().find("storage_user") != std::string::npos, "status-user-storage");
    auto doc = app.doctor();
    CHECK_TRUE(doc.find("runtime_paths") != std::string::npos, "doctor");

    coreai::CoreApp restarted;
    auto ri = restarted.initialize(root.string());
    CHECK_TRUE(ri.ok(), "restart-initialize");
    if (ri.ok()) {
        auto rm = restarted.memorySearch("full-product");
        CHECK_TRUE(okText(rm), "restart-memory");
        if (rm.ok()) CHECK_TRUE(rm.value().find("CORE full-product memory") != std::string::npos, "restart-memory-content");
        auto rp = restarted.projectList();
        CHECK_TRUE(okText(rp), "restart-projects");
        if (rp.ok()) CHECK_TRUE(rp.value().find("Demo") != std::string::npos, "restart-project-content");
        auto rl = restarted.recent();
        CHECK_TRUE(okText(rl), "restart-recent");
        if (rl.ok() && c.ok() && reset.ok()) CHECK_TRUE(rl.value().find(c.value()) != std::string::npos || rl.value().find(reset.value()) != std::string::npos, "restart-recent-content");
    }


    auto userRoot = root / "users-test";
#ifdef _WIN32
    _putenv_s("CORE_USER_STORAGE_QUOTA_MB", "16");
#else
    setenv("CORE_USER_STORAGE_QUOTA_MB", "16", 1);
#endif
    auto alice = coreai::UserStorage::resolve(userRoot, "alice");
    CHECK_TRUE(alice.ok(), "user-storage-create");
    if (alice.ok()) {
        CHECK_TRUE(alice.value().user_id == "alice", "user-storage-user-id");
        CHECK_TRUE(alice.value().shard_index == 1, "user-storage-first-shard");
        std::filesystem::create_directories(alice.value().shard_root);
        std::ofstream(alice.value().shard_root / "quota.bin") << std::string(17 * 1024 * 1024, 'x');
        auto rolled = coreai::UserStorage::ensure_capacity(alice.value(), 0);
        CHECK_TRUE(rolled.ok(), "user-storage-rollover-create");
        if (rolled.ok()) {
            CHECK_TRUE(rolled.value().shard_index == 2, "user-storage-rollover-index");
            CHECK_TRUE(std::filesystem::exists(rolled.value().shard_root), "user-storage-rollover-folder");
        }
        auto bob = coreai::UserStorage::resolve(userRoot, "bob");
        CHECK_TRUE(bob.ok(), "user-storage-second-user");
        if (bob.ok()) CHECK_TRUE(bob.value().user_root != alice.value().user_root, "user-storage-user-isolation");
    }

    } // destroy CoreApp/database owners before deleting the disposable test root

    for (int attempt = 0; attempt < 8; ++attempt) {
        std::error_code cleanup_ec;
        std::filesystem::remove_all(root, cleanup_ec);
        if (!std::filesystem::exists(root)) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(100 * (attempt + 1)));
    }
    if (failures != 0) {
        std::cerr << "CORE-AI LOCAL TESTS FAILED: " << failures << std::endl;
        return 1;
    }
    std::cout << "ALL LOCAL CORE-AI TESTS PASSED" << std::endl;
    return 0;
}
