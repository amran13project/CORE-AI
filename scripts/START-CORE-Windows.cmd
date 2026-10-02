@echo off
cd /d "%~dp0.."
call scripts\BUILD-No-CMake-Windows.cmd
if errorlevel 1 exit /b 1
build-native\core-ai.exe
