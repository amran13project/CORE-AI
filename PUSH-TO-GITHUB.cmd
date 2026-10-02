@echo off
setlocal
cd /d "%~dp0"

echo ============================================
echo [CORE] GITHUB PUSH
echo ============================================

where git >nul 2>nul
if errorlevel 1 (
  echo [CORE] Git was not found in PATH.
  echo [CORE] Nothing was installed by this script.
  exit /b 1
)

if not exist ".git" (
  echo [CORE] Initializing Git repository...
  git init -b main
  if errorlevel 1 exit /b 1
)

git remote get-url origin >nul 2>nul
if errorlevel 1 git remote add origin https://github.com/amran13project/CORE-AI.git

git add .
if errorlevel 1 exit /b 1

git diff --cached --quiet
if errorlevel 1 (
  git commit -m "feat: add CORE AI foundation"
  if errorlevel 1 exit /b 1
) else (
  echo [CORE] No new changes to commit.
)

echo.
echo [CORE] Pushing to GitHub...
echo [CORE] GitHub may ask you to sign in through your configured credential helper.
git push -u origin main
if errorlevel 1 (
  echo.
  echo [CORE] Push failed. No software was installed by this script.
  exit /b 1
)

echo.
echo [CORE] Push successful.
exit /b 0
