# CORE-AI Windows Build Fix

This fix avoids CMake entirely and uses an already-installed clang++. It also links WinHTTP for the Windows Ollama provider.

Copy these two files into `scripts/` in your extracted CORE-AI folder, then run:

`cmd /c ".\scripts\Build-CORE-AI-Windows-NoCMake.cmd"`

The script searches PATH and common existing LLVM-MinGW/ForgeEngine toolchain locations. It does not install software.
