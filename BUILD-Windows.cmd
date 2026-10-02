@echo off
setlocal
cd /d "%~dp0"

where cmake >nul 2>nul
if errorlevel 1 (
  if exist "%ProgramFiles%\CMake\bin\cmake.exe" (
    set "PATH=%ProgramFiles%\CMake\bin;%PATH%"
  ) else if exist "%ProgramFiles(x86)%\CMake\bin\cmake.exe" (
    set "PATH=%ProgramFiles(x86)%\CMake\bin;%PATH%"
  ) else if exist "%LOCALAPPDATA%\Programs\CMake\bin\cmake.exe" (
    set "PATH=%LOCALAPPDATA%\Programs\CMake\bin;%PATH%"
  ) else (
    echo [CORE] CMake was not found.
    echo [CORE] Install CMake from https://cmake.org/download/ and reopen this terminal.
    exit /b 1
  )
)

where ninja >nul 2>nul
if errorlevel 1 (
  if exist "%USERPROFILE%\scoop\apps\ninja\current\ninja.exe" set "PATH=%USERPROFILE%\scoop\apps\ninja\current;%PATH%"
)

where ninja >nul 2>nul
if errorlevel 1 (
  echo [CORE] Ninja not found; using CMake default generator.
  cmake -S . -B build
  if errorlevel 1 exit /b 1
  cmake --build build --config Release
  if errorlevel 1 exit /b 1
  ctest --test-dir build -C Release --output-on-failure
) else (
  cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
  if errorlevel 1 exit /b 1
  cmake --build build
  if errorlevel 1 exit /b 1
  ctest --test-dir build --output-on-failure
)

if errorlevel 1 exit /b 1
echo [CORE] Windows build + tests passed.
exit /b 0
