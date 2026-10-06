## 0.3.2

- Fix No-CMake Windows source list to include `src/core/UserStorage.cpp`.
- Prevent stale CORE-AI build processes from locking the previous executable during rebuild.
- Add visible UI working state with a moving runner/obstacle activity animation; this is presentation feedback only.
- Add explicit chat error cards with stable error codes and retry metadata.
- Preserve compact independent sidebar scrolling and separate chat scrolling.

## 0.3.1

- Compact independently scrollable sidebar navigation.
- Separate chat/workspace scrolling from navigation.
- Per-user storage isolation with quota-aware shard rollover.
- Added storage user/shard information to status output.

# Changelog

## 0.3.0-v14
- Increase Ollama network receive/send timeout to 120 seconds by default.
- Support `CORE_OLLAMA_TIMEOUT_MS` for bounded configuration.
- Preserve truthful provider errors instead of failing fast during normal local model startup.


## 0.3.0

- Rebuilt the core around one `coreai::CoreApp` authority.
- Added runtime paths and SQLite persistence.
- Added truthful capability reporting.
- Added loopback API and served web client.
- Added Ollama local HTTP adapter.
- Added CLI diagnostics and persistence commands.

## 0.3.0-v13
- Replaced the Windows Ollama transport with direct loopback TCP HTTP.
- Added explicit Winsock error reporting for provider connection failures.
- Removed the provider's dependency on WinHTTP/proxy behavior for local Ollama.
- Preserved the truthful unavailable state when the endpoint is actually unreachable.
