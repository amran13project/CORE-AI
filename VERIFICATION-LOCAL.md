# CORE-AI Local Verification

Version: 0.3.0

Build environment: Linux x64, GCC 14.2.0, CMake 3.31.6, Ninja, CTest.

## Verified

- Clean CMake configure: PASS
- Clean C++20 build: PASS
- CTest: 1/1 PASS
- CLI version/status/capabilities: PASS
- SQLite runtime and schema initialization: PASS
- New Chat persistence: PASS
- Recent persistence after process restart: PASS
- Memory write/search persistence after process restart: PASS
- Project creation/list persistence after process restart: PASS
- Project traversal rejection: PASS
- Loopback `/health`: PASS
- Loopback `/status`: PASS
- Loopback `/capabilities`: PASS
- Web `/`: PASS
- Memory API URL decoding: PASS
- Unavailable Ollama behavior: PASS (reported as unavailable; no fabricated response)
- `/models` with Ollama absent: PASS as truthful UNAVAILABLE response
- Unimplemented capability endpoints: PASS as truthful NOT_IMPLEMENTED responses
- Developer path scan: PASS
- Secret pattern scan: PASS

## Not verified

- Windows x64 native build
- Win32 GUI compile/runtime
- Windows package creation/extraction/launch
- Ollama live generation
- remote providers
- maps backend
- image generation
- voice
- vision
- semantic retrieval
- Codex repository automation
- Git service
- agent runtime
- multi-agent runtime
- workflow execution
- scheduler execution
- plugin execution

## Truth rule

No Windows release ZIP is produced from this environment because doing so would violate the product specification's verification requirements.
