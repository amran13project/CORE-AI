#include "services/CoreApp.h"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <thread>
#include <vector>

namespace {

std::filesystem::path findGuiRoot(const char* argv0) {
    std::vector<std::filesystem::path> candidates;

    std::error_code ec;

    candidates.push_back(std::filesystem::current_path() / "gui");

    if (argv0 && *argv0) {
        auto exe = std::filesystem::absolute(argv0, ec);

        if (!ec) {
            const auto dir = exe.parent_path();

            candidates.push_back(dir / "gui");
            candidates.push_back(dir.parent_path() / "gui");
            candidates.push_back(dir.parent_path().parent_path() / "gui");
            candidates.push_back(dir.parent_path().parent_path().parent_path() / "gui");
        }
    }

    for (const auto& candidate : candidates) {
        if (std::filesystem::exists(candidate / "index.html", ec) && !ec) {
            return std::filesystem::absolute(candidate, ec);
        }
    }

    return {};
}

}

int main(int argc, char** argv) {
    const auto guiRoot = findGuiRoot(argc > 0 ? argv[0] : nullptr);

    if (guiRoot.empty()) {
        std::cerr << "CORE-AI GUI folder not found.\n";
        std::cerr << "Expected: <project>\\gui\\index.html\n";
        return 1;
    }

    const auto projectRoot = guiRoot.parent_path();

    coreai::CoreApp app;

    const auto init = app.initialize(projectRoot.string());

    if (!init.ok()) {
        std::cerr << "CORE-AI initialization failed.\n";
        return 2;
    }

    if (argc > 1) {
        const std::string cmd = argv[1];

        if (cmd == "status") {
            const auto result = app.status();
            if (result.ok()) {
                std::cout << result.value() << '\n';
                return 0;
            }

            std::cerr << "Status failed.\n";
            return 3;
        }

        if (cmd == "capabilities") {
            std::cout << app.capabilities() << '\n';
            return 0;
        }

        if ((cmd == "chat" || cmd == "think" ||
             cmd == "code" || cmd == "agent") && argc > 2) {

            std::string prompt;

            for (int i = 2; i < argc; ++i) {
                if (!prompt.empty()) {
                    prompt += ' ';
                }
                prompt += argv[i];
            }

            const std::string mode =
                cmd == "think" ? "think" :
                cmd == "code"  ? "code"  :
                cmd == "agent" ? "agent" :
                                  "chat";

            const auto result = app.chat(prompt, mode);

            if (result.ok()) {
                std::cout << result.value() << '\n';
                return 0;
            }

            std::cerr << "Chat failed.\n";
            return 4;
        }
    }

    const auto api = app.startApi(guiRoot);

    if (!api.ok()) {
        std::cerr << "CORE-AI API server failed to start.\n";
        return 5;
    }

    std::cout
        << "CORE AI 0.2\n"
        << "GUI root: " << guiRoot.string() << '\n'
        << "GUI: http://127.0.0.1:47821/\n"
        << "Press Ctrl+C to stop.\n";

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
