# CORE-AI v19 fix

Fixed the Windows no-CMake linker failure for `coreai::UserStorage::resolve(...)` by adding `src/core/UserStorage.cpp` to the native application and test source list.

Also stops stale `core-ai.exe` processes before relinking to avoid Windows file-lock failures.
