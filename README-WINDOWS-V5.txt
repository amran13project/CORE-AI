CORE-AI Windows Full v11

This release fixes a Windows test-isolation bug where the disposable SQLite runtime root
was deleted while a CoreApp/Database owner was still alive. The integration test now
keeps all CoreApp instances inside a scope and performs cleanup only after their
SQLite handles and mapped DLLs are destroyed.

Run from the project root:
  cmd /c ".\\scripts\\Build-CORE-AI-Windows-NoCMake.cmd"

No CMake installation is required by this script. It reuses an existing C++ compiler.
