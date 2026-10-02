@echo off
setlocal EnableExtensions EnableDelayedExpansion
cd /d "%~dp0"

call "%~dp0BUILD-No-CMake-Windows.cmd"
if errorlevel 1 exit /b 1

echo.
echo [CORE] Starting CORE AI...

rem Add the existing LLVM-MinGW runtime to PATH for this process only.
for /d %%D in ("%USERPROFILE%\Desktop\ForgeEngine\llvm-mingw-*" "%USERPROFILE%\Desktop\ForgeEngine\Toolchain\llvm-mingw-*") do (
  if exist "%%~fD\bin\clang++.exe" set "PATH=%%~fD\bin;!PATH!"
)
"%~dp0build-native\core-ai.exe"
set "CORE_EXIT=%ERRORLEVEL%"
echo [CORE] CORE AI exited with code %CORE_EXIT%.
if not "%CORE_EXIT%"=="0" echo [CORE] Startup failed. Check the Windows error dialog or run the executable directly.
exit /b %CORE_EXIT%
