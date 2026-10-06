@echo off
setlocal
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0Build-CORE-AI-Windows.ps1"
if errorlevel 1 exit /b 1
