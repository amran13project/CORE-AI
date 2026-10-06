CORE-AI Windows Full v4

This build fixes the Windows 0xC0000135 / -1073741515 test-launch failure.

The previous script could find clang++.exe by absolute path while the compiler's
runtime DLL directory was not inherited by the test process. v4 adds the compiler
runtime directory to PATH, stages common runtime DLLs before tests, and uses
-static-libgcc / -static-libstdc++ where supported.

Run:
  cmd /c ".\\scripts\\Build-CORE-AI-Windows-NoCMake.cmd"

Do not run the old v3 script.

V5 FIX
The no-CMake Windows builder now creates the runtime directory before staging DLLs.
It only copies runtime DLLs that actually exist and adds the staged runtime directory to PATH before tests.
