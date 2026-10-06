@echo off
setlocal
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Build-CORE-AI-Windows-NoCMake.ps1"
exit /b %errorlevel%
