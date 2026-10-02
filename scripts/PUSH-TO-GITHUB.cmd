@echo off
cd /d "%~dp0.."
if not exist .git git init -b main
git add .
git diff --cached --quiet && echo [CORE] No new changes. && exit /b 0
git commit -m "feat: expand CORE AI full platform"
git push origin main
