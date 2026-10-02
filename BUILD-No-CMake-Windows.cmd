@echo off
setlocal EnableExtensions EnableDelayedExpansion
cd /d "%~dp0"

echo ============================================
echo [CORE] BUILD WITHOUT CMAKE
echo ============================================

set "CXX="

rem 1) Compiler already on PATH.
where clang++.exe >nul 2>nul
if not errorlevel 1 set "CXX=clang++.exe"
if not defined CXX (
  where g++.exe >nul 2>nul
  if not errorlevel 1 set "CXX=g++.exe"
)

rem 2) Reuse LLVM-MinGW already present in common project/toolchain folders.
rem    This does NOT install anything. It only finds an existing compiler.
if not defined CXX (
  for /d %%D in ("%USERPROFILE%\Desktop\llvm-mingw-*") do (
    if exist "%%~fD\bin\clang++.exe" (
      set "CXX=%%~fD\bin\clang++.exe"
      goto :compiler_found
    )
    if exist "%%~fD\bin\g++.exe" (
      set "CXX=%%~fD\bin\g++.exe"
      goto :compiler_found
    )
  )
)

if not defined CXX (
  for /d %%D in ("%USERPROFILE%\Desktop\ForgeEngine\llvm-mingw-*") do (
    if exist "%%~fD\bin\clang++.exe" (
      set "CXX=%%~fD\bin\clang++.exe"
      goto :compiler_found
    )
    if exist "%%~fD\bin\g++.exe" (
      set "CXX=%%~fD\bin\g++.exe"
      goto :compiler_found
    )
  )
)

if not defined CXX (
  for /d %%D in ("%USERPROFILE%\Desktop\ForgeEngine\Toolchain\llvm-mingw-*") do (
    if exist "%%~fD\bin\clang++.exe" (
      set "CXX=%%~fD\bin\clang++.exe"
      goto :compiler_found
    )
    if exist "%%~fD\bin\g++.exe" (
      set "CXX=%%~fD\bin\g++.exe"
      goto :compiler_found
    )
  )
)

:compiler_found
if not defined CXX (
  echo [CORE] No C++ compiler was found.
  echo [CORE] This no-CMake build uses an existing clang++ or g++ installation.
  echo [CORE] No software was installed by this script.
  exit /b 1
)

echo [CORE] Compiler: %CXX%

for %%I in ("%CXX%") do set "TOOLCHAIN_BIN=%%~dpI"
if not defined TOOLCHAIN_BIN set "TOOLCHAIN_BIN="

if exist build-native rmdir /s /q build-native
mkdir build-native
mkdir build-native\obj

set CPPFLAGS=-std=c++20 -O2 -I"%~dp0src" -DCORE_HAS_WIN32_GUI=1
set "APP_CPP=src\main.cpp"
for /r src %%F in (*.cpp) do (
  if /i not "%%~fF"=="%~dp0src\main.cpp" if /i not "%%~fF"=="%~dp0tests\core_tests.cpp" (
    set "APP_CPP=!APP_CPP! "%%~fF""
  )
)

rem Build the main application.
"%CXX%" %CPPFLAGS% -mwindows -static %APP_CPP% -o build-native\core-ai.exe -luser32 -lgdi32 -ladvapi32 -lshell32
if errorlevel 1 (
  echo [CORE] Application build FAILED.
  exit /b 1
)

rem Build the native test executable using all library sources, excluding main.cpp and the test itself.
set "TEST_CPP=tests\core_tests.cpp"
for /r src %%F in (*.cpp) do (
  if /i not "%%~fF"=="%~dp0src\main.cpp" (
    set "TEST_CPP=!TEST_CPP! "%%~fF""
  )
)

"%CXX%" %CPPFLAGS% -static %TEST_CPP% -o build-native\core-ai-tests.exe -luser32 -lgdi32 -ladvapi32 -lshell32
if errorlevel 1 (
  echo [CORE] Test build FAILED.
  exit /b 1
)

build-native\core-ai-tests.exe
if errorlevel 1 (
  echo [CORE] Tests FAILED.
  exit /b 1
)

echo.

rem Keep the compiler runtime available beside the executable.
if defined TOOLCHAIN_BIN (
  for %%D in (libc++.dll libc++abi.dll libunwind.dll libwinpthread-1.dll libgcc_s_seh-1.dll) do (
    if exist "!TOOLCHAIN_BIN!%%D" copy /y "!TOOLCHAIN_BIN!%%D" "build-native\%%D" >nul
  )
)

echo [CORE] CMake-free Windows build + tests passed.
echo [CORE] Executable: %~dp0build-native\core-ai.exe
exit /b 0
