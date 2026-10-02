#include "ai/OllamaProvider.h"
#include <cstdio>
#include <array>
#include <memory>
#include <sstream>
#include <sys/types.h>

namespace {
std::string shellQuote(const std::string& s) {
#if defined(_WIN32)
    std::string out = "\\\"";
    for (char c : s) { if (c == '\\' || c == '\"') out += '\\'; out += c; }
    out += "\\\""; return out;
#else
    std::string out = "'";
    for (char c : s) { if (c == '\'') out += "'\\''"; else out += c; }
    out += "'"; return out;
#endif
}
}
namespace core::ai {
OllamaProvider::OllamaProvider(std::string model) : model_(std::move(model)) {}
AIResponse OllamaProvider::complete(const AIRequest& request) {
    std::string prompt = request.system.empty() ? request.prompt : (request.system + "\n\nUSER:\n" + request.prompt);
    const std::string command = "ollama run " + shellQuote(model_) + " " + shellQuote(prompt) + " 2>&1";
#if defined(_WIN32)
    FILE* pipe = _popen(command.c_str(), "r");
#else
    FILE* pipe = popen(command.c_str(), "r");
#endif
    if (!pipe) return {false, {}, "Unable to start Ollama. Is it installed and on PATH?", -1};
    std::array<char, 4096> buffer{};
    std::string output;
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe)) output += buffer.data();
#if defined(_WIN32)
    const int rc = _pclose(pipe);
#else
    const int rc = pclose(pipe);
#endif
    if (rc != 0) return {false, output, "Ollama returned a non-zero exit code.", rc};
    return {true, output, {}, rc};
}
}
