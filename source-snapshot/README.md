# CORE-AI

YOUR INTELLIGENCE. YOUR SYSTEM. YOUR CONTROL.

CORE-AI is a native C++20 local-first personal AI application with one service authority shared by CLI, loopback HTTP API, and the web control surface. The project is deliberately truthful: external features are reported unavailable or not implemented when their actual dependency/implementation is missing.

## Main commands

`core-ai version`
`core-ai status`
`core-ai capabilities`
`core-ai new-chat`
`core-ai reset-chat`
`core-ai recent`
`core-ai chat "hello"`
`core-ai think "solve..."`
`core-ai codex "inspect..."`
`core-ai memory add "text"`
`core-ai memory search "query"`
`core-ai projects create "Demo"`
`core-ai projects list`
`core-ai serve`

## Runtime

By default CORE-AI uses a per-user application data root. Pass `--root <path>` before a command to use an isolated runtime root. Structured state uses SQLite when the runtime library is available.

## AI provider

Ollama is supported through its local HTTP API at `http://127.0.0.1:11434`. Configure `model=` and `provider.ollama.url=` in the runtime configuration. No fake model or response is generated when the provider is offline.

## Web

`core-ai serve` starts the loopback API and web client at `http://127.0.0.1:47821/`.
