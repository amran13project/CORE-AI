# CORE-AI 0.3.0 — Verification Status

Verified in the current Linux x64 engineering environment.

## PASS
- Clean CMake configure
- Clean C++20 build
- CTest
- CLI version/status/capabilities/doctor
- SQLite persistence
- Conversations: new/recent/reset
- Memory persistence/search
- Projects persistence/path traversal protection
- Local file sandbox read/write
- TXT/MD/JSON/CSV document inspection
- Library artifact persistence
- Local provenance-aware library research
- Persistent tasks
- Persistent workflow definitions + execution
- Persistent schedules + manual run
- Bounded local planner agent
- Bounded planner/reviewer/tester multi-agent orchestration
- Native plugin discovery
- Real Git status against a temporary Git repository
- Local image artifact generation (SVG)
- Maps deep-link adapter
- Loopback API routes and POST body parsing
- Web client loading from the local API
- Restart persistence
- Local path traversal rejection

## UNAVAILABLE / NOT VERIFIED
- Ollama runtime: endpoint not running in this environment
- Live model discovery/generation: depends on a reachable AI provider
- Think runtime: depends on live AI generation
- Remote provider runtime: not configured
- Vision backend: no verified vision model/backend present
- Voice backend: no verified STT/TTS backend present
- Semantic embeddings: no embedding backend present
- Full Codex repository patch/build/test orchestration: not implemented in this snapshot
- Native Win32 GUI runtime: requires Windows execution environment
- Windows x64 build/package: requires a Windows toolchain/runtime environment
- Final Windows ZIP release: therefore NOT_VERIFIED

No external capability is represented as PASS unless it was actually exercised here.
