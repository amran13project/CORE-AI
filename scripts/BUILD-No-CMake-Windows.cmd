@echo off
setlocal EnableExtensions
cd /d "%~dp0.."
set "CXX="
where clang++.exe >nul 2>nul && set "CXX=clang++.exe"
if not defined CXX if exist "%USERPROFILE%\Desktop\ForgeEngine\llvm-mingw-20260922-msvcrt-x86_64\bin\clang++.exe" set "CXX=%USERPROFILE%\Desktop\ForgeEngine\llvm-mingw-20260922-msvcrt-x86_64\bin\clang++.exe"
if not defined CXX echo [CORE] clang++ not found. & exit /b 1
if exist build-native rmdir /s /q build-native
mkdir build-native
"%CXX%" -std=c++20 -O2 -Iinclude -Isrc src\main.cpp src\core\CoreApp.cpp src\core\Config.cpp src\core\Logger.cpp src\core\EventBus.cpp src\ai\ModelRouter.cpp src\ai\OllamaProvider.cpp src\memory\MemoryStore.cpp src\tools\ToolRegistry.cpp src\tools\FileTool.cpp src\tools\ProcessTool.cpp src\agents\Planner.cpp src\agents\AgentManager.cpp src\security\PermissionManager.cpp src\audit\AuditLog.cpp src\verification\VerificationEngine.cpp src\projects\ProjectManager.cpp src\automation\WorkflowEngine.cpp src\plugins\PluginManager.cpp src\server\LocalApiServer.cpp -o build-native\core-ai.exe -mconsole -static-libgcc -static-libstdc++ -lws2_32 -lshell32 -ladvapi32 -luser32
if errorlevel 1 exit /b 1
"%CXX%" -std=c++20 -O2 -Iinclude -Isrc tests\core_tests.cpp src\core\Config.cpp src\memory\MemoryStore.cpp src\security\PermissionManager.cpp src\verification\VerificationEngine.cpp src\agents\Planner.cpp src\projects\ProjectManager.cpp -o build-native\core-ai-tests.exe
if errorlevel 1 exit /b 1
build-native\core-ai-tests.exe
if errorlevel 1 exit /b 1
echo [CORE] Build + tests passed.
