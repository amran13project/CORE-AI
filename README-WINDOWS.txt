CORE-AI Windows x64 build

This package uses C++20 and an existing LLVM/MinGW-compatible compiler.
It does not require CMake for the NoCMake build path.
The build script stages an official SQLite Windows x64 DLL as a runtime dependency.


## Windows runtime note (v0.3.0-v7)
The Windows executable uses the console subsystem so CLI output remains visible while the same executable can start the native Win32 GUI. `core-ai serve` remains running until the process is interrupted with Ctrl+C.
