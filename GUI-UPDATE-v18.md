# CORE-AI v0.3.2 GUI Update v18

Fixes the Windows native GUI compile error caused by mixing a wide string literal with `std::string` in the error path. The conversion is now performed on a UTF-8 `std::string` before `toW()`.

Also includes the v17 GUI composer behavior:
- compact independently scrollable sidebar
- separate chat scroll area
- Enter sends
- Shift+Enter inserts a newline
- Think button near the composer
- + menu for Plugins, Tools, Files, Images and Library
- microphone action with truthful browser Speech Recognition fallback
- Function menu for Think/Codex/Maps/Research/New Chat/Reset Chat
- working/error activity UI
