# Build

Requirements: C++20 compiler, CMake 3.20+, CTest. Ninja is optional.

Linux/macOS: `cmake -S . -B build -G Ninja` then `cmake --build build` and `ctest --test-dir build --output-on-failure`.

Windows: use `scripts/Build-CORE-AI-Windows.ps1` or the `.cmd` wrapper. The script does not install software.

Windows native GUI is compiled with Win32 when `COREAI_BUILD_NATIVE_GUI=ON`.


## Windows runtime note (v0.3.0-v7)
The Windows executable uses the console subsystem so CLI output remains visible while the same executable can start the native Win32 GUI. `core-ai serve` remains running until the process is interrupted with Ctrl+C.
