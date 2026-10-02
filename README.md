# CORE AI — Full Personal AI Platform

CORE AI is a self-built C++20 AI platform with a replaceable HTML/CSS/JavaScript GUI. The project is designed to grow beyond a single chat screen by composing models, tools, memory, agents, projects, plugins, automation, verification, and security into one local-first system.

## Included

- `CORE-AI.cpp` — single-file native core/API foundation.
- `src/` — modular production architecture.
- `include/` — public interfaces and GUI-facing C ABI header.
- `gui/` — replaceable HTML/CSS/JS interface.
- `plugins/` — plugin manifest + example SDK source.
- `tests/` — real unit/integration foundation.
- `docs/` — architecture, capability, security, plugin rules.
- `scripts/` — Windows build/start/push helpers.

## Windows without CMake

The included script reuses the LLVM-MinGW compiler already present on the machine. It does not install software.

`powershell`

`cmd /c scripts\\BUILD-No-CMake-Windows.cmd`

## Important

Advanced capabilities are only marked implemented when the underlying code/backend exists. Cloud AI, image generation, voice, browser automation, embeddings, and other advanced integrations remain adapters or roadmap items until connected and verified.

## GUI

Edit `gui/index.html`, `gui/style.css`, and `gui/app.js` freely. The GUI contract is documented in `gui/API.md`.
