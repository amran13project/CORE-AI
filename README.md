# CORE AI

**Personal Intelligence & Creation Platform — self-built foundation**

CORE AI is a C++20, CMake-based local-first AI workspace designed to be extended by its owner. The foundation is intentionally modular: brain/orchestration, memory, tools, agents, verification, security, plugins, projects, and automation are separate systems.

## What is real in v0.1

- Native C++20 application and cross-platform core.
- Local model adapter for Ollama via the `ollama` command.
- Model router abstraction for future providers.
- File tool with read/write.
- Process tool with explicit permission checks.
- Persistent JSONL-like personal/project memory.
- Agent/task planner using deterministic plans plus optional local-model assistance.
- Plugin discovery and loading interface on Windows (`LoadLibrary`) and POSIX (`dlopen`).
- Workflow engine for event/action chains.
- Project manager with project-local `.core` data.
- Verification engine with real command exit-code verification.
- Audit log.
- Native Win32 GUI on Windows, console UI elsewhere.
- Example plugin SDK project.

## Intentionally not fake

The following are extension points, not claimed implementations: image generation, full browser automation, speech synthesis, advanced vision models, remote cloud providers, and autonomous unrestricted computer control. Their interfaces can be added without pretending the capability exists.

## Local model

Install Ollama, install a model, then run:

```text
core-ai chat "Hello CORE"
```

The default model is configurable in `core.config` or with `CORE_MODEL`.

## Build on Windows

Use a recent CMake + C++20 compiler (LLVM-MinGW, MinGW, or Visual Studio). Example:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
.\build\core-ai.exe doctor
.\build\core-ai.exe chat "Hello CORE"
```

## Build on Linux/macOS

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

## Safety model

Potentially destructive capabilities are permission-gated. CORE records important actions in `audit.log` and does not report build/test success unless the underlying process returned success.

## Roadmap

1. Local model streaming and structured tool calling.
2. SQLite memory backend.
3. RAG/embeddings backends.
4. Native code editor + canvas workspace.
5. Whisper-based voice adapter.
6. Vision adapter.
7. Plugin UI panels and signed manifests.
8. Agent evaluation harness.
9. Project graph / dependency indexing.
10. Sandboxed process runner.

This repository is a foundation for a serious personal AI system, not a claim that every advanced AI capability is already implemented.

## Windows build troubleshooting

CORE AI does not require PowerShell execution-policy changes when launched with `START-CORE-Windows.cmd` or `BUILD-Windows.cmd`.

The Windows scripts first look for CMake in PATH and common installation locations. If CMake is not installed, install the free official CMake package from https://cmake.org/download/ and reopen the terminal.

## Windows without CMake

The no-CMake launcher can reuse an existing LLVM-MinGW compiler. It checks:

- compiler already on PATH
- `Desktop\llvm-mingw-*`
- `Desktop\ForgeEngine\llvm-mingw-*`
- `Desktop\ForgeEngine\Toolchain\llvm-mingw-*`

No compiler or other software is installed by the launcher.

Run:

```powershell
.\START-CORE-Windows.cmd
```


## Startup diagnostics
The native Windows launcher validates Win32 startup and reports the Windows error code if window creation fails.

## Windows startup/runtime
The launcher reuses an existing LLVM-MinGW compiler under `Desktop\ForgeEngine` and does not install software. The Windows application is linked with static runtime support when available and the launcher adds the detected toolchain `bin` directory to PATH for this process only. Runtime DLLs present in that toolchain are also copied beside the executable.
